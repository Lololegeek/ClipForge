#include "Updater.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QFileInfo>
#include <QHash>
#include <QPointer>
#include <QProcess>
#include <QTextStream>
#include <QThread>
#include <QUrl>
#include <QVersionNumber>
#include <QtZlib/zlib.h>
#include <algorithm>
#include <memory>
#include <windows.h>
namespace cf {
namespace {
const char *kDefaultLatest = "https://api.github.com/repos/Lololegeek/ClipForge/releases/latest";
const char *kManifest = "ClipForge-manifest.json";
// CLIPFORGE_UPDATE_API lets tests point the updater at a local server.
QString latestUrl() {
    const auto env = qEnvironmentVariable("CLIPFORGE_UPDATE_API");
    return env.isEmpty() ? QString(kDefaultLatest) : env;
}
const char *kAsset = "ClipForge-Setup-x64.exe";
QNetworkRequest request(const QUrl &url) {
    QNetworkRequest r(url);
    r.setHeader(QNetworkRequest::UserAgentHeader, "ClipForge-Updater");
    r.setRawHeader("Accept", "application/vnd.github+json");
    r.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    r.setTransferTimeout(30000);
    return r;
}
// Manifest paths come from the network: refuse anything that could escape the staging folder.
bool safeRelative(const QString &p) {
    if (p.isEmpty() || p.startsWith('/') || p.contains('\\') || p.contains(':'))
        return false;
    for (const auto &part : p.split('/'))
        if (part.isEmpty() || part == "." || part == "..")
            return false;
    return true;
}
bool trustedUrl(const QString &u) {
    return u.startsWith("https://") ||
           (!qEnvironmentVariable("CLIPFORGE_UPDATE_API").isEmpty() && u.startsWith("http://127.0.0.1"));
}
// Inflates a raw Deflate stream (a ZIP entry); returns an empty array unless it yields exactly `size` bytes.
QByteArray inflateRaw(const QByteArray &in, qint64 size) {
    QByteArray out(size, Qt::Uninitialized);
    z_stream s{};
    if (inflateInit2(&s, -MAX_WBITS) != Z_OK)
        return {};
    s.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(in.constData()));
    s.avail_in = uInt(in.size());
    s.next_out = reinterpret_cast<Bytef *>(out.data());
    s.avail_out = uInt(out.size());
    const int rc = inflate(&s, Z_FINISH);
    const bool ok = rc == Z_STREAM_END && qint64(s.total_out) == size;
    inflateEnd(&s);
    return ok ? out : QByteArray();
}
QByteArray sha256File(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return {};
    QCryptographicHash h(QCryptographicHash::Sha256);
    h.addData(&f);
    return h.result().toHex();
}
} // namespace
Updater::Updater(QString data, QObject *parent)
    : QObject(parent), net_(new QNetworkAccessManager(this)), data_(std::move(data)) {
    // An installer left over from a previous update is stale by definition.
    QDir(data_ + "/update").removeRecursively();
}
int Updater::compare(const QString &a, const QString &b) {
    auto parse = [](QString v) {
        if (v.startsWith('v', Qt::CaseInsensitive))
            v.remove(0, 1);
        return QVersionNumber::fromString(v);
    };
    return QVersionNumber::compare(parse(a), parse(b));
}
bool Updater::selfUpdatable() const {
    return QFile::exists(QCoreApplication::applicationDirPath() + "/unins000.exe");
}
void Updater::setState(const QString &s) {
    state_ = s;
    emit changed();
}
void Updater::fail(const QString &text) {
    setState("idle");
    if (manual_)
        emit message(text, true);
}
void Updater::check(bool manual, bool download) {
    if (state_ == "checking" || state_ == "downloading" || state_ == "ready")
        return;
    manual_ = manual;
    download_ = download;
    setState("checking");
    auto reply = net_->get(request(QUrl(latestUrl())));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError)
            return fail("Impossible de vérifier les mises à jour : " + reply->errorString());
        auto o = QJsonDocument::fromJson(reply->readAll()).object();
        QString tag = o["tag_name"].toString();
        if (tag.isEmpty() || o["draft"].toBool() || o["prerelease"].toBool())
            return fail("Réponse de mise à jour invalide.");
        if (compare(tag, QCoreApplication::applicationVersion()) <= 0) {
            setState("idle");
            if (manual_)
                emit message("ClipForge est à jour (" + QCoreApplication::applicationVersion() + ").", false);
            return;
        }
        version_ = tag.startsWith('v') ? tag.mid(1) : tag;
        page_ = o["html_url"].toString();
        notes_ = o["body"].toString().trimmed();
        url_.clear();
        digest_.clear();
        manifestUrl_.clear();
        assets_.clear();
        installerSize_ = 0;
        for (auto a : o["assets"].toArray()) {
            auto asset = a.toObject();
            const QString name = asset["name"].toString();
            assets_.insert(name, {asset["browser_download_url"].toString(), qint64(asset["size"].toDouble())});
            if (name == kManifest)
                manifestUrl_ = asset["browser_download_url"].toString();
            if (name == kAsset) {
                installerSize_ = qint64(asset["size"].toDouble());
                url_ = asset["browser_download_url"].toString();
                digest_ = asset["digest"].toString();
            }
        }
        downloadSize_ = installerSize_;
        if (!trustedUrl(url_))
            return fail("Aucun installateur trouvé pour la version " + version_ + ".");
        setState("available");
        if (download_ && selfUpdatable())
            startDownload();
        else
            emit message("Mise à jour " + version_ + " disponible.", false);
    });
}
void Updater::downloadNow() {
    if (state_ != "available")
        return;
    if (!selfUpdatable()) {
        QDesktopServices::openUrl(QUrl(page_));
        return;
    }
    manual_ = true;
    startDownload();
}
void Updater::startDownload() {
    QDir().mkpath(data_ + "/update");
    delta_ = false;
    if (!manifestUrl_.isEmpty())
        return startDelta();
    startFullDownload();
}
void Updater::startFullDownload() {
    file_ = data_ + "/update/" + kAsset;
    progress_ = 0;
    downloadSize_ = installerSize_;
    setState("downloading");
    auto out = std::make_shared<QFile>(file_ + ".part");
    if (!out->open(QIODevice::WriteOnly | QIODevice::Truncate))
        return fail("Impossible d'enregistrer la mise à jour.");
    auto hash = std::make_shared<QCryptographicHash>(QCryptographicHash::Sha256);
    auto reply = net_->get(request(QUrl(url_)));
    connect(reply, &QNetworkReply::readyRead, this, [reply, out, hash] {
        auto chunk = reply->readAll();
        hash->addData(chunk);
        out->write(chunk);
    });
    connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 got, qint64 total) {
        progress_ = total > 0 ? double(got) / double(total) : 0;
        emit changed();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, out, hash] {
        reply->deleteLater();
        out->close();
        if (reply->error() != QNetworkReply::NoError) {
            out->remove();
            return fail("Téléchargement de la mise à jour impossible : " + reply->errorString());
        }
        if (out->size() < 1024 * 1024) {
            out->remove();
            return fail("Installateur téléchargé invalide.");
        }
        if (digest_.startsWith("sha256:") && hash->result().toHex() != digest_.mid(7).toLatin1()) {
            out->remove();
            return fail("Somme de contrôle de la mise à jour incorrecte.");
        }
        QFile::remove(file_);
        if (!out->rename(file_))
            return fail("Impossible d'enregistrer la mise à jour.");
        setState("ready");
        emit message("Mise à jour " + version_ + " prête : elle s'installera à la fermeture ou via Paramètres.", false);
    });
}
void Updater::startDelta() {
    progress_ = 0;
    setState("downloading");
    auto reply = net_->get(request(QUrl(manifestUrl_)));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError)
            return startFullDownload();
        const auto doc = QJsonDocument::fromJson(reply->readAll()).object();
        // The manifest describes the release ZIP; its bytes are fetched with Range requests on that asset.
        const auto archive = assets_.value(doc["archive"].toString());
        const qint64 archiveSize = qint64(doc["archiveSize"].toDouble());
        if (!trustedUrl(archive.first) || archiveSize <= 0 || archive.second != archiveSize)
            return startFullDownload();
        archiveUrl_ = archive.first;
        std::vector<Entry> all;
        for (const auto &v : doc["files"].toArray()) {
            const auto o = v.toObject();
            Entry e{o["path"].toString(), o["sha256"].toString().toLower(), qint64(o["size"].toDouble()),
                    qint64(o["csize"].toDouble()), qint64(o["offset"].toDouble()), o["method"].toInt(-1)};
            if (!safeRelative(e.path) || e.sha.size() != 64 || e.size < 0 || e.csize < 0 || e.offset <= 0 ||
                e.offset + e.csize > archiveSize || (e.method != 0 && e.method != 8) ||
                e.size > 512LL * 1024 * 1024)
                return startFullDownload();
            all.push_back(std::move(e));
        }
        if (all.empty())
            return startFullDownload();
        const qint64 installerSize = qint64(doc["installerSize"].toDouble());
        // Hash the installed files off the UI thread, then keep only what differs.
        const QString root = QCoreApplication::applicationDirPath();
        QPointer<Updater> self(this);
        QThread *t = QThread::create([self, all, root, installerSize]() mutable {
            std::vector<Entry> changed;
            for (auto &e : all) {
                QFileInfo info(root + "/" + e.path);
                if (info.exists() && info.size() == e.size && sha256File(info.filePath()) == e.sha.toLatin1())
                    continue;
                changed.push_back(std::move(e));
            }
            std::sort(changed.begin(), changed.end(), [](const Entry &a, const Entry &b) { return a.offset < b.offset; });
            // Neighbouring files share one request: a small gap costs less than another round trip.
            constexpr qint64 kGap = 256 * 1024, kMaxJob = 32LL * 1024 * 1024;
            std::vector<Job> missing;
            qint64 total = 0;
            for (auto &e : changed) {
                if (!missing.empty()) {
                    auto &last = missing.back();
                    const qint64 end = last.start + last.length;
                    if (e.offset - end <= kGap && e.offset + e.csize - last.start <= kMaxJob) {
                        total += e.offset + e.csize - end;
                        last.length = e.offset + e.csize - last.start;
                        last.files.push_back(std::move(e));
                        continue;
                    }
                }
                total += e.csize;
                missing.push_back({e.offset, e.csize, {}});
                missing.back().files.push_back(std::move(e));
            }
            if (!self)
                return;
            QMetaObject::invokeMethod(
                self.data(),
                [self, missing = std::move(missing), total, installerSize]() mutable {
                    if (!self)
                        return;
                    // A delta that is not clearly smaller than the installer is not worth the risk.
                    if (missing.empty() || (installerSize > 0 && total > installerSize * 7 / 10))
                        return self->startFullDownload();
                    self->missing_ = std::move(missing);
                    self->deltaTotal_ = total;
                    self->downloadSize_ = total;
                    self->deltaDone_ = 0;
                    QDir(self->data_ + "/update/stage").removeRecursively();
                    self->delta_ = true;
                    self->downloadNextBlob(0);
                },
                Qt::QueuedConnection);
        });
        connect(t, &QThread::finished, t, &QObject::deleteLater);
        t->start();
    });
}
void Updater::downloadNextBlob(size_t index) {
    if (index >= missing_.size()) {
        progress_ = 1;
        setState("ready");
        emit message("Mise à jour " + version_ + " prête (" + QString::number(deltaTotal_ / 1048576.0, 'f', 1) +
                         " Mo seulement) : elle s'installera à la fermeture ou via Paramètres.",
                     false);
        return;
    }
    const Job job = missing_[index];
    auto abandon = [this](const QString &why) {
        qWarning().noquote() << "delta update failed:" << why << "- falling back to the full installer";
        QDir(data_ + "/update/stage").removeRecursively();
        delta_ = false;
        startFullDownload();
    };
    // Unpacks the files of a downloaded range into the staging folder, after checking each one.
    auto stage = [this, job](const QByteArray &bytes) -> QString {
        for (const auto &e : job.files) {
            const QByteArray packed = bytes.mid(int(e.offset - job.start), int(e.csize));
            const QByteArray data = e.method == 8 ? inflateRaw(packed, e.size) : packed;
            if (packed.size() != e.csize || data.size() != e.size ||
                QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex() != e.sha.toLatin1())
                return "checksum mismatch for " + e.path;
            const QString target = data_ + "/update/stage/" + e.path;
            QDir().mkpath(QFileInfo(target).absolutePath());
            QFile f(target);
            if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate) || f.write(data) != data.size())
                return "cannot write " + e.path;
        }
        return {};
    };
    if (job.length == 0) { // empty files only: nothing to download
        if (const auto err = stage({}); !err.isEmpty())
            return abandon(err);
        return downloadNextBlob(index + 1);
    }
    QNetworkRequest r = request(QUrl(archiveUrl_));
    r.setRawHeader("Accept", "application/octet-stream");
    r.setRawHeader("Range", QByteArray("bytes=") + QByteArray::number(job.start) + '-' +
                                QByteArray::number(job.start + job.length - 1));
    auto reply = net_->get(r);
    const qint64 before = deltaDone_;
    connect(reply, &QNetworkReply::downloadProgress, this, [this, before, reply, job](qint64 got, qint64) {
        // A server that ignores Range would send the whole archive: stop before wasting the bandwidth.
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200 || got > job.length) {
            reply->abort();
            return;
        }
        progress_ = deltaTotal_ > 0 ? double(before + got) / double(deltaTotal_) : 0;
        emit changed();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, job, index, abandon, stage] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError)
            return abandon(reply->errorString());
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 206)
            return abandon("server ignored the Range request");
        const QByteArray bytes = reply->readAll();
        if (bytes.size() != job.length)
            return abandon("short range");
        if (const auto err = stage(bytes); !err.isEmpty())
            return abandon(err);
        deltaDone_ += job.length;
        downloadNextBlob(index + 1);
    });
}
bool Updater::launchDelta() {
    const QString stage = QDir::toNativeSeparators(QDir(data_ + "/update/stage").absolutePath());
    const QString app = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
    const QString script = QDir::toNativeSeparators(QDir::temp().absoluteFilePath("clipforge-apply.cmd"));
    QFile f(script);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    // Waits for this process to exit (files are locked while it runs), copies the staged files, relaunches.
    f.write("@echo off\r\n"
            "set PID=%~1\r\nset STAGE=%~2\r\nset APP=%~3\r\nset DATA=%~4\r\nset N=0\r\n"
            ":wait\r\n"
            "tasklist /FI \"PID eq %PID%\" 2>NUL | \"%SystemRoot%\\System32\\find.exe\" \"%PID%\" >NUL\r\n"
            "if not errorlevel 1 (ping -n 2 127.0.0.1 >NUL & goto wait)\r\n"
            ":copy\r\n"
            "xcopy /E /Y /Q \"%STAGE%\\*\" \"%APP%\\\" >NUL && goto done\r\n"
            "set /a N+=1\r\n"
            "if %N% GEQ 10 goto done\r\n"
            "ping -n 3 127.0.0.1 >NUL\r\n"
            "goto copy\r\n"
            ":done\r\n"
            "rd /s /q \"%STAGE%\" >NUL 2>&1\r\n"
            "start \"\" \"%APP%\\ClipForge.exe\" --headless --data-dir \"%DATA%\"\r\n"
            "exit /b 0\r\n");
    f.close();
    QProcess p;
    p.setProgram(qEnvironmentVariable("ComSpec", "cmd.exe"));
    const QString data = QDir::toNativeSeparators(QDir(data_).absolutePath());
    p.setNativeArguments(QString("/c \"\"%1\" %2 \"%3\" \"%4\" \"%5\"\"")
                             .arg(script)
                             .arg(QCoreApplication::applicationPid())
                             .arg(stage, app, data));
    p.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *a) { a->flags |= CREATE_NO_WINDOW; });
    return p.startDetached();
}
bool Updater::launchInstaller() {
    if (state_ == "ready" && delta_)
        return launchDelta();
    if (state_ != "ready" || !QFile::exists(file_))
        return false;
    return QProcess::startDetached(file_, {"/VERYSILENT", "/SUPPRESSMSGBOXES", "/NORESTART", "/CLOSEAPPLICATIONS", "/relaunch=1"});
}
} // namespace cf
