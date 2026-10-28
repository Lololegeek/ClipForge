#include "Controller.h"
#include "Overlay.h"
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
#include <QSettings>
#include <QScreen>
#include <QStorageInfo>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QUrl>
#include <windows.h>
namespace cf {
void Controller::start() {
    if (!ready_ || !(state_ == "Idle" || state_ == "Error"))
        return;
    state_ = "Initializing";
    emit changed();
    auto s = settings_;
    QMetaObject::invokeMethod(engine_, [this, s] { engine_->start(s); });
}
void Controller::stop() {
    QMetaObject::invokeMethod(engine_, &ObsEngine::stop);
}
void Controller::saveReplay() {
    requestClip(settings_.clipSeconds);
}
void Controller::saveFull() {
    requestClip(0);
}
void Controller::requestClip(double seconds) {
    if (state_ != "Buffering" && state_ != "Recording" && state_ != "Saving") {
        notify("Activez d'abord l'Instant Replay.", true);
        toast("Replay inactif", "Activez l'Instant Replay pour clipper.", Overlay::Error);
        return;
    }
    // One clip at a time and at most one request per 1.5 s: mashing the key must not queue dozens of files.
    if (state_ == "Saving" || !gate(saveGate_, 1500))
        return;
    const bool shortened = seconds > 0 && seconds < settings_.replaySeconds;
    pendingClips_ << PendingClip{shortened ? seconds : 0, QDateTime::currentMSecsSinceEpoch()};
    toast("Clip en cours…",
          shortened ? QString("Sauvegarde des %1 dernières secondes").arg(int(seconds)) : "Sauvegarde de tout le tampon",
          Overlay::Progress);
    QMetaObject::invokeMethod(engine_, &ObsEngine::saveReplay);
}
void Controller::clipSaved(QString p) {
    if (p.endsWith(".mkv", Qt::CaseInsensitive)) {
        // Manual recordings (self-test) are kept whole and carry no markers.
        lastClip_ = p;
        jobs_.import(p, game_, true);
        emit changed();
        return;
    }
    const auto clip = pendingClips_.isEmpty() ? PendingClip{0, QDateTime::currentMSecsSinceEpoch()}
                                              : pendingClips_.takeFirst();
    const double window = clip.keep > 0 ? clip.keep : settings_.replaySeconds;
    QVariantList offsets;
    for (auto m : markers_)
        if (m <= clip.at && clip.at - m <= window * 1000)
            offsets << double(clip.at - m) / 1000.0;
    clipMarkers_[QFileInfo(p).absoluteFilePath()] = offsets;
    if (settings_.saveSound)
        MessageBeep(MB_OK);
    notify("Clip sauvegardé : " + QFileInfo(p).fileName());
    toast("Clip sauvegardé", QFileInfo(p).fileName(), Overlay::Success);
    if (clip.keep > 0) {
        jobs_.trimTail(p, clip.keep);
        return;
    }
    lastClip_ = p;
    jobs_.import(p, game_, true);
    emit changed();
}
void Controller::clipImported(QVariantMap data) {
    const auto it = clipMarkers_.find(QFileInfo(data["path"].toString()).absoluteFilePath());
    if (it == clipMarkers_.end()) {
        library_.upsert(data);
        return;
    }
    // Markers were measured back from the key press, which is the end of the saved file.
    const double duration = data["duration"].toDouble();
    QJsonArray markers;
    for (const auto &offset : *it) {
        const double at = duration - offset.toDouble();
        if (at >= 0)
            markers << std::round(at * 100) / 100;
    }
    clipMarkers_.erase(it);
    if (!markers.isEmpty())
        data["markers"] = QString::fromUtf8(QJsonDocument(markers).toJson(QJsonDocument::Compact));
    library_.upsert(data);
    cleanupLibrary();
}
void Controller::addMarker() {
    if (state_ != "Buffering" && state_ != "Recording" && state_ != "Saving") {
        toast("Replay inactif", "Activez l'Instant Replay pour poser un marqueur.", Overlay::Error);
        return;
    }
    const auto now = QDateTime::currentMSecsSinceEpoch();
    markers_.removeIf([&](qint64 m) { return now - m > (settings_.replaySeconds + 10) * 1000LL; });
    markers_ << now;
    toast("Marqueur posé", "Il apparaîtra sur la timeline du prochain clip", Overlay::Info);
}
void Controller::setMicMuted(bool muted) {
    micMuted_ = muted;
    QMetaObject::invokeMethod(engine_, [this, muted] { engine_->setMicMuted(muted); });
    emit changed();
}
void Controller::micKey() {
    if (settings_.microphone == "disabled")
        return;
    if (settings_.pushToTalk) {
        setMicMuted(false);
        pttTimer_.start();
        return;
    }
    setMicMuted(!micMuted_);
    toast(micMuted_ ? "Micro coupé" : "Micro actif", {}, micMuted_ ? Overlay::Error : Overlay::Info);
}
void Controller::configureHotkeys() {
    auto e = hotkeys_.configure(settings_.saveKey, settings_.markerKey, settings_.micKey);
    if (!e.isEmpty())
        notify(e, true);
}
int Controller::cleanupLibrary() {
    const int removed = library_.cleanup(qint64(settings_.cleanupGB) * 1073741824LL, settings_.cleanupDays);
    if (removed > 0)
        notify(QString("Nettoyage : %1 ancien(s) clip(s) envoyé(s) à la corbeille.").arg(removed));
    return removed;
}
void Controller::cleanupNow() {
    if (settings_.cleanupGB <= 0 && settings_.cleanupDays <= 0) {
        notify("Aucune limite de nettoyage n'est définie.", true);
        return;
    }
    if (cleanupLibrary() == 0)
        notify("Rien à nettoyer : la bibliothèque respecte les limites.");
}
void Controller::toggleRecording() {
    if (!gate(recordGate_, 1500))
        return;
    if (state_ == "Idle" || state_ == "Error")
        start();
    QMetaObject::invokeMethod(engine_, &ObsEngine::toggleRecording);
}
bool Controller::apply(QVariantMap v) {
    const bool running = state_ == "Buffering" || state_ == "Recording";
    if (!running && state_ != "Idle" && state_ != "Error") {
        notify("Patientez un instant : la capture est occupée (démarrage ou sauvegarde).", true);
        return false;
    }
    auto s = Settings::fromJson(QJsonObject::fromVariantMap(v));
    QString e;
    if (!s.save(data_ + "/settings.json", &e)) {
        notify(e, true);
        return false;
    }
    // Options that only affect the app never need the capture pipeline to be rebuilt.
    auto captureKey = [](const Settings &x) {
        auto o = x.json();
        for (const char *k : {"closeToTray", "autoCapture", "startMinimized", "gameOverlay", "autoUpdate", "alwaysReplay",
                              "saveKey", "markerKey", "micKey", "clipSeconds", "cleanupGB", "cleanupDays",
                              "pushToTalk", "saveSound", "profiles"})
            o.remove(k);
        return o;
    };
    const bool restart = running && captureKey(s) != captureKey(settings_);
    const bool micChanged = s.pushToTalk != settings_.pushToTalk || s.micKey != settings_.micKey;
    settings_ = s;
    emit configChanged();
    e = hotkeys_.configure(s.saveKey, s.markerKey, s.micKey);
    if (!e.isEmpty())
        notify(e, true);
    else
        notify("Réglages enregistrés.");
    if (micChanged && running) {
        pttTimer_.stop();
        setMicMuted(s.pushToTalk);
    }
    cleanupLibrary();
    emit changed();
    if (restart) {
        // Rebuild the replay live: stop it, and the Idle transition below starts it again with the new values.
        restartPending_ = true;
        notify("Réglages enregistrés : redémarrage du replay…");
        stop();
        return true;
    }
    // Idle/Error: bring the replay straight back with the new values.
    if (!running && settings_.alwaysReplay && !test_ && !uiOnly_)
        QTimer::singleShot(150, this, &Controller::start);
    return true;
}
bool Controller::allowClose() const {
    return quitting_ || !settings_.closeToTray;
}
void Controller::quit() {
    if (jobs_.busy() || state_ == "Saving") {
        notify("Attendez la fin de la sauvegarde ou de l'export avant de quitter.", true);
        return;
    }
    // Never wait forever for the engine to go idle: a stuck state must not keep the process alive.
    if (state_ != "Idle" && state_ != "Error" && quitAttempts_++ < 15) {
        stop();
        QTimer::singleShot(400, this, &Controller::quit);
        return;
    }
    quitting_ = true;
    emit changed();
    updater_.launchInstaller();
    QApplication::quit();
}
static const char *kRunKey = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
bool Controller::autostart() const {
    return QSettings(kRunKey, QSettings::NativeFormat).contains("ClipForge");
}
void Controller::setAutostart(bool enabled) {
    QSettings run(kRunKey, QSettings::NativeFormat);
    if (enabled)
        run.setValue("ClipForge", "\"" + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\" --headless");
    else
        run.remove("ClipForge");
    emit changed();
}
void Controller::checkForUpdates() {
    updater_.check(true, false);
}
void Controller::installUpdate() {
    if (updater_.state() == "available")
        updater_.downloadNow();
    else if (updater_.ready())
        quit();
    else
        updater_.check(true, true);
}
void Controller::runSelfTest() {
    int duration = 32000;
    auto args = QCoreApplication::arguments();
    int i = args.indexOf("--test-seconds");
    if (i >= 0 && i + 1 < args.size())
        duration = std::clamp(args[i + 1].toInt(), 32, 3600) * 1000;
    if (duration > 60000)
        settings_.replaySeconds = 60;
    if (args.contains("--test-1080")) {
        settings_.width = 1920;
        settings_.height = 1080;
        settings_.fps = 60;
        settings_.bitrate = 16000;
    }
    if (args.contains("--software-test"))
        settings_.encoder = "obs_x264";
    if (args.contains("--window-test")) {
        settings_.mode = "window";
        settings_.target = "ClipForge Capture Fixture:ClipForgeFixture:ClipForgeFixture.exe";
        settings_.audio = "disabled";
    }
    start();
    QTimer::singleShot(duration - 14000, this, &Controller::saveReplay);
    QTimer::singleShot(duration - 13800, this, &Controller::saveReplay);
    // Exercise the rapid-keypress guard, then save again after its cooldown.
    QTimer::singleShot(duration - 12000, this, &Controller::saveReplay);
    QTimer::singleShot(duration - 10000, this, &Controller::toggleRecording);
    QTimer::singleShot(duration - 5000, this, &Controller::toggleRecording);
    QTimer::singleShot(duration - 3000, this, &Controller::stop);
    QTimer::singleShot(duration, this, [this] {
        QFile f(data_ + "/metrics.json");
        if (f.open(QIODevice::WriteOnly))
            f.write(QJsonDocument(QJsonObject::fromVariantMap(metrics_)).toJson());
        QFile samples(data_ + "/samples.json");
        if (samples.open(QIODevice::WriteOnly))
            samples.write(QJsonDocument(testSamples_).toJson());
        quitting_ = true;
        QApplication::quit();
    });
}
} // namespace cf
