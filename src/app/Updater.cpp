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
#include <QProcess>
#include <QUrl>
#include <QVersionNumber>
namespace cf {
namespace {
const char *kLatest = "https://api.github.com/repos/Lololegeek/ClipForge/releases/latest";
const char *kAsset = "ClipForge-Setup-x64.exe";
QNetworkRequest request(const QUrl &url) {
    QNetworkRequest r(url);
    r.setHeader(QNetworkRequest::UserAgentHeader, "ClipForge-Updater");
    r.setRawHeader("Accept", "application/vnd.github+json");
    r.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    r.setTransferTimeout(30000);
    return r;
}
} // namespace
Updater::Updater(QString data, QObject *parent)
    : QObject(parent), net_(new QNetworkAccessManager(this)), data_(std::move(data)) {}
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
    auto reply = net_->get(request(QUrl(kLatest)));
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
        url_.clear();
        digest_.clear();
        for (auto a : o["assets"].toArray()) {
            auto asset = a.toObject();
            if (asset["name"].toString() == kAsset) {
                url_ = asset["browser_download_url"].toString();
                digest_ = asset["digest"].toString();
            }
        }
        if (!url_.startsWith("https://"))
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
    file_ = data_ + "/update/" + kAsset;
    progress_ = 0;
    setState("downloading");
    auto reply = net_->get(request(QUrl(url_)));
    reply->setReadBufferSize(0);
    connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 got, qint64 total) {
        progress_ = total > 0 ? double(got) / double(total) : 0;
        emit changed();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError)
            return fail("Téléchargement de la mise à jour impossible : " + reply->errorString());
        auto bytes = reply->readAll();
        if (bytes.size() < 1024 * 1024)
            return fail("Installateur téléchargé invalide.");
        if (digest_.startsWith("sha256:") &&
            QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex() != digest_.mid(7).toLatin1())
            return fail("Somme de contrôle de la mise à jour incorrecte.");
        QFile f(file_);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate) || f.write(bytes) != bytes.size())
            return fail("Impossible d'enregistrer la mise à jour.");
        f.close();
        setState("ready");
        emit message("Mise à jour " + version_ + " prête : elle s'installera à la fermeture ou via Paramètres.", false);
    });
}
bool Updater::launchInstaller() {
    if (state_ != "ready" || !QFile::exists(file_))
        return false;
    return QProcess::startDetached(file_, {"/VERYSILENT", "/SUPPRESSMSGBOXES", "/NORESTART", "/CLOSEAPPLICATIONS", "/relaunch=1"});
}
} // namespace cf
