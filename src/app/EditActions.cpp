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
void Controller::exportClip(int height, int fps, QString codec, double volume) {
    auto dest = QFileDialog::getSaveFileName(
        nullptr, "Exporter en MP4",
        settings_.clips + "/Export " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss") + ".mp4",
        "MP4 (*.mp4)");
    if (dest.isEmpty())
        return;
    if (!dest.endsWith(".mp4", Qt::CaseInsensitive))
        dest += ".mp4";
    auto project = project_;
    project.volume = volume;
    progress_ = 0;
    jobs_.exportVideo(project, dest, height, fps, codec);
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