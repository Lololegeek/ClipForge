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
#include <QPainter>
#include <QProcess>
#include <QScreen>
#include <QStorageInfo>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QUrl>
namespace cf {
void Controller::scan() {
    QDir dir(settings_.clips);
    auto entries = dir.entryInfoList({"*.mp4", "*.mkv", "*.mov", "*.webm"}, QDir::Files, QDir::Time);
    // Unchanged clips that already have a thumbnail are not probed again on every launch.
    auto known = library_.knownFiles();
    QFileInfoList fresh;
    for (const auto &f : entries) {
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
    emit catalogChanged();
}
void Controller::importVideos() {
    for (auto p : QFileDialog::getOpenFileNames(nullptr, "Importer des vidéos", {},
                                                "Vidéos (*.mp4 *.mkv *.mov *.webm *.avi)"))
        jobs_.import(p);
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
void Controller::copyPath(QString p) {
    QApplication::clipboard()->setText(p);
    notify("Chemin copié.");
}
} // namespace cf