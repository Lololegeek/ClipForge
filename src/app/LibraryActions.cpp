#include "Controller.h"
#include "platform/WindowCatalog.h"
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMenu>
#include <QHttpMultiPart>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPainter>
#include <QProcess>
#include <QSet>
#include <QScreen>
#include <QStorageInfo>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QUrl>
#include <QUuid>
namespace cf {
void Controller::scan() {
    if (jobs_.pendingImports() > 0)
        return; // an earlier scan is still importing
    QDir dir(settings_.clips);
    auto entries = dir.entryInfoList({"*.mp4", "*.mkv", "*.mov", "*.webm"}, QDir::Files, QDir::Time);
    // Unchanged clips that already have a thumbnail are not probed again on every launch.
    auto known = library_.knownFiles();
    QFileInfoList fresh;
    const auto now = QDateTime::currentDateTime();
    for (const auto &f : entries) {
        if (f.lastModified().secsTo(now) < 6) // still being written
            continue;
        auto it = known.constFind(f.absoluteFilePath());
        if (it == known.cend() || it.value() != f.size())
            fresh << f;
    }
    if (fresh.isEmpty())
        return;
    auto work = std::make_shared<QFileInfoList>(fresh);
    auto timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, this, [this, timer, work] {
        if (jobs_.pendingImports() > 128)
            return;
        int n = 0;
        while (!work->isEmpty() && n++ < 8) {
            auto f = work->takeFirst();
            jobs_.import(f.absoluteFilePath());
        }
        if (work->isEmpty()) {
            timer->stop();
            timer->deleteLater();
        }
    });
    timer->start();
}
void Controller::refresh() {
    library_.reload();
    scan();
    refreshWindows();
}
void Controller::refreshWindows() {
    games_ = windows();
    QVariantList targets;
    for (auto t : targets_)
        if (t.toMap()["mode"] == "display")
            targets << t;
    for (auto w : games_) {
        auto m = w.toMap();
        targets << QVariantMap{{"mode", "window"}, {"name", m["title"]}, {"value", m["selector"]}};
    }
    targets_ = targets;
    // Games page: one row per program (not per window), saved profiles kept even when the game is closed.
    QVariantList games, others;
    QSet<QString> seen;
    for (auto w : games_) {
        const auto m = w.toMap();
        const QString exe = m["executable"].toString();
        if (seen.contains(exe.toLower()))
            continue;
        seen.insert(exe.toLower());
        const bool profile = stored().profiles.contains(exe.toLower());
        const bool game = profile || looksLikeGame(exe, m["fullscreen"].toBool());
        const QVariantMap row{{"title", programTitle(exe)}, {"executable", exe}, {"running", true}, {"game", game}};
        (game ? games : others) << row;
    }
    for (auto it = stored().profiles.begin(); it != stored().profiles.end(); ++it)
        if (!seen.contains(it.key()))
            games << QVariantMap{{"title", it.value().toObject()["name"].toString()},
                                 {"executable", it.key()},
                                 {"running", false},
                                 {"game", true}};
    gameList_ = games + others;
    emit catalogChanged();
}
void Controller::importVideos() {
    for (auto p : QFileDialog::getOpenFileNames(nullptr, "Importer des vidéos", {},
                                                "Vidéos (*.mp4 *.mkv *.mov *.webm *.avi)"))
        jobs_.import(p);
}
void Controller::importFiles(QStringList urls) {
    static const QStringList ok{"mp4", "mkv", "mov", "webm", "avi"};
    int count = 0;
    for (const auto &u : urls) {
        const QString p = QUrl(u).isLocalFile() ? QUrl(u).toLocalFile() : u;
        if (QFileInfo(p).isFile() && ok.contains(QFileInfo(p).suffix().toLower())) {
            jobs_.import(p);
            ++count;
        }
    }
    notify(count ? QString("%1 vidéo(s) en cours d'import.").arg(count) : "Aucune vidéo reconnue dans ce dépôt.", !count);
}
void Controller::selectClip(int i) {
    selected_ = library_.get(i);
    if (selected_.isEmpty())
        return;
    project_ = {};
    project_.source = selected_["path"].toString();
    project_.segments = {{0, selected_["duration"].toDouble(), 1}};
    // Probe audio asynchronously before export; source recordings always have AAC, imports may be silent.
    auto probe = new QProcess(this);
    connect(probe, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, probe, path = project_.source](int, QProcess::ExitStatus) {
                if (project_.source == path) {
                    // One line per audio stream: 3 means mix + game + microphone, editable separately.
                    const auto lines = QString::fromUtf8(probe->readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);
                    int tracks = 0;
                    for (const auto &l : lines)
                        tracks += !l.trimmed().isEmpty();
                    project_.hasAudio = tracks > 0;
                    project_.audioTracks = std::max(1, tracks);
                    emit editChanged();
                }
                probe->deleteLater();
            });
    probe->start(QFileInfo(QCoreApplication::applicationDirPath() + "/tools/ffprobe.exe").exists()
                     ? QCoreApplication::applicationDirPath() + "/tools/ffprobe.exe"
                     : "ffprobe",
                 {"-v", "error", "-select_streams", "a", "-show_entries", "stream=index", "-of", "csv=p=0",
                  project_.source});
    undo_.clear();
    redo_.clear();
    emit selectionChanged();
    emit editChanged();
    emit openEditor();
}
void Controller::reveal(QString path) {
    if (path.isEmpty())
        path = settings_.clips;
    QProcess::startDetached("explorer.exe", QFileInfo(path).isDir()
                                                ? QStringList{QDir::toNativeSeparators(path)}
                                                : QStringList{"/select,", QDir::toNativeSeparators(path)});
}
void Controller::copyFile(QString p) {
    if (!QFileInfo::exists(p)) {
        notify("Fichier introuvable.", true);
        return;
    }
    auto *mime = new QMimeData;
    mime->setUrls({QUrl::fromLocalFile(p)});
    QApplication::clipboard()->setMimeData(mime);
    notify("Fichier copié : colle-le directement dans Discord.");
}
// Webhooks take up to 25 MB on most servers; some (no boost) stop at 10 MB, which a 413 reply tells us.
static constexpr qint64 kDiscordBytes = 25LL * 1024 * 1024, kDiscordSmallBytes = 10LL * 1024 * 1024;
static bool validHook(const QString &hook) {
    return hook.startsWith("https://discord.com/api/webhooks/") || hook.startsWith("https://discordapp.com/api/webhooks/");
}
void Controller::sendToDiscord(QString p) {
    uploadToDiscord(p, p, false, false);
}
void Controller::autoDiscord(const QVariantMap &clip) {
    const QString p = clip["path"].toString();
    if (!validHook(stored().discordWebhook.trimmed())) {
        notify("Envoi Discord automatique : le webhook dans les réglages n'est pas valide.", true);
        return;
    }
    discordDurations_[p] = clip["duration"].toDouble();
    if (QFileInfo(p).size() <= kDiscordBytes)
        uploadToDiscord(p, p, true, false);
    else
        shrinkForDiscord(p, 24);
}
void Controller::shrinkForDiscord(const QString &clip, double maxMB) {
    // A smaller copy is made in the temp folder, sent, then deleted. The clip itself is untouched.
    const QString temp = QDir::tempPath() + "/clipforge-discord-" + QUuid::createUuid().toString(QUuid::Id128) + ".mp4";
    jobs_.shrink(clip, discordDurations_.value(clip), maxMB, temp);
}
void Controller::uploadToDiscord(QString p, QString clip, bool automatic, bool removeAfter) {
    const QString hook = stored().discordWebhook.trimmed();
    if (!validHook(hook)) {
        notify("Renseigne d'abord un webhook Discord valide dans les réglages.", true);
        if (removeAfter)
            QFile::remove(p);
        return;
    }
    auto *file = new QFile(p);
    if (!file->exists() || file->size() > kDiscordBytes || !file->open(QIODevice::ReadOnly)) {
        notify(file->exists() && file->size() > kDiscordBytes ? "Clip trop lourd pour Discord (25 Mo max) : exporte-le avec le préréglage Discord."
                                                              : "Fichier introuvable.", true);
        delete file;
        if (removeAfter)
            QFile::remove(p);
        return;
    }
    const qint64 bytes = file->size();
    auto *multi = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart part;
    part.setHeader(QNetworkRequest::ContentTypeHeader, "video/mp4");
    part.setHeader(QNetworkRequest::ContentDispositionHeader,
                   QString("form-data; name=\"file\"; filename=\"%1\"").arg(QFileInfo(clip).completeBaseName() + ".mp4"));
    part.setBodyDevice(file);
    file->setParent(multi);
    if (removeAfter) // ~QFile has closed the handle by the time destroyed() fires
        connect(file, &QObject::destroyed, [p] { QFile::remove(p); });
    multi->append(part);
    auto *net = new QNetworkAccessManager(this);
    auto *reply = net->post(QNetworkRequest(QUrl(hook)), multi);
    multi->setParent(reply);
    if (!automatic)
        notify("Envoi sur Discord…");
    connect(reply, &QNetworkReply::finished, this, [this, reply, net, clip, automatic, bytes] {
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (automatic && status == 413 && bytes > kDiscordSmallBytes) {
            // This server only takes 10 MB: retry with a smaller copy.
            shrinkForDiscord(clip, 9.5);
        } else if (reply->error() == QNetworkReply::NoError) {
            discordDurations_.remove(clip);
            notify(automatic ? "Clip envoyé sur Discord : " + QFileInfo(clip).fileName() + " (cliquez pour l'ouvrir)"
                             : "Clip envoyé sur Discord.");
            if (automatic)
                notifyClip_ = clip;
        } else {
            discordDurations_.remove(clip);
            notify(status == 413 ? "Envoi Discord échoué : clip trop lourd pour ce serveur."
                                 : "Envoi Discord échoué : " + reply->errorString(),
                   true);
        }
        reply->deleteLater();
        net->deleteLater();
    });
}
void Controller::copyPath(QString p) {
    QApplication::clipboard()->setText(p);
    notify("Chemin copié.");
}
} // namespace cf