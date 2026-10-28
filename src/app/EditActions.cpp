#include "Controller.h"
#include "platform/WindowCatalog.h"
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QImage>
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
QVariantList Controller::segments() const {
    QVariantList l;
    for (auto s : project_.segments)
        l << QVariantMap{{"in", s.in}, {"out", s.out}, {"speed", s.speed}};
    return l;
}
void Controller::checkpoint() {
    undo_ << project_;
    if (undo_.size() > 100)
        undo_.removeFirst();
    redo_.clear();
}
void Controller::addSegment(double in, double out, double speed) {
    if (in < 0 || out <= in || out > selected_["duration"].toDouble() + 0.1 ||
        project_.segments.size() >= 128) {
        notify("Choisissez un passage dans la durée de la vidéo.", true);
        return;
    }
    checkpoint();
    project_.segments << Segment{in, out, speed};
    emit editChanged();
}
void Controller::useSegment(double in, double out, double speed) {
    if (in < 0 || out <= in || out > selected_["duration"].toDouble() + 0.1) {
        notify("Passage invalide.", true);
        return;
    }
    checkpoint();
    project_.segments = {{in, out, speed}};
    emit editChanged();
}
void Controller::removeSegment(int i) {
    if (i < 0 || i >= project_.segments.size())
        return;
    checkpoint();
    project_.segments.remove(i);
    emit editChanged();
}
void Controller::undo() {
    if (undo_.isEmpty())
        return;
    redo_ << project_;
    project_ = undo_.takeLast();
    emit editChanged();
}
void Controller::redo() {
    if (redo_.isEmpty())
        return;
    undo_ << project_;
    project_ = redo_.takeLast();
    emit editChanged();
}
QVariantMap Controller::edit() const {
    return {{"audioTracks", project_.audioTracks},
            {"crop", QVariantList{project_.cropX, project_.cropY, project_.cropW, project_.cropH}},
            {"cropped", project_.cropped()}};
}
void Controller::setCrop(double x, double y, double w, double h) {
    w = std::clamp(w, 0.05, 1.0);
    h = std::clamp(h, 0.05, 1.0);
    x = std::clamp(x, 0.0, 1.0 - w);
    y = std::clamp(y, 0.0, 1.0 - h);
    if (x == project_.cropX && y == project_.cropY && w == project_.cropW && h == project_.cropH)
        return;
    checkpoint();
    project_.cropX = x;
    project_.cropY = y;
    project_.cropW = w;
    project_.cropH = h;
    emit editChanged();
}
void Controller::exportClip(QVariantMap o) {
    ExportOptions options;
    options.format = o.value("format", "mp4").toString();
    options.height = o.value("height", 0).toInt();
    options.fps = o.value("fps", 60).toInt();
    options.codec = o.value("codec", "h264").toString();
    options.crf = o.value("crf", 20).toInt();
    options.maxMB = o.value("maxMB", 0).toDouble();
    options.vertical = o.value("vertical", false).toBool();
    const QString ext = options.format == "mp4" ? "mp4" : options.format;
    const QString label = ext == "gif" ? "GIF animé (*.gif)" : ext == "webp" ? "WebP animé (*.webp)" : "MP4 (*.mp4)";
    auto dest = QFileDialog::getSaveFileName(
        nullptr, "Exporter",
        settings_.clips + "/Export " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss") + "." + ext,
        label);
    if (dest.isEmpty())
        return;
    if (!dest.endsWith("." + ext, Qt::CaseInsensitive))
        dest += "." + ext;
    auto project = project_;
    project.volume = o.value("volume", 1.0).toDouble();
    project.gameVolume = o.value("gameVolume", 1.0).toDouble();
    project.micVolume = o.value("micVolume", 1.0).toDouble();
    progress_ = 0;
    lastExport_.clear();
    jobs_.exportVideo(project, dest, options);
}
void Controller::captureFrame(double seconds) {
    if (project_.source.isEmpty())
        return;
    const QString dest = settings_.clips + "/Captures/" + QFileInfo(project_.source).completeBaseName() + " " +
                         QString::number(seconds, 'f', 2).replace('.', ',') + "s.png";
    jobs_.snapshot(project_.source, std::max(0.0, seconds), dest);
}
void Controller::saveProject() {
    auto path =
        QFileDialog::getSaveFileName(nullptr, "Enregistrer le montage", {}, "Projet ClipForge (*.clipforge)");
    if (!path.isEmpty()) {
        if (!path.endsWith(".clipforge"))
            path += ".clipforge";
        notify(project_.save(path) ? "Projet enregistré." : "Échec de l'enregistrement du projet.");
    }
}
void Controller::loadProject() {
    auto path =
        QFileDialog::getOpenFileName(nullptr, "Ouvrir un montage", {}, "Projet ClipForge (*.clipforge)");
    if (path.isEmpty())
        return;
    auto p = EditProject::load(path);
    auto e = p.validate();
    if (!e.isEmpty()) {
        notify(e, true);
        return;
    }
    checkpoint();
    project_ = p;
    selected_ = {{"path", p.source}, {"name", QFileInfo(p.source).completeBaseName()}, {"duration", 0}};
    double end = 0;
    for (auto s : p.segments)
        end = std::max(end, s.out);
    selected_["duration"] = end;
    emit selectionChanged();
    emit editChanged();
    emit openEditor();
}
} // namespace cf