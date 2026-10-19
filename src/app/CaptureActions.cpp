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
#include <QSettings>
#include <QScreen>
#include <QStorageInfo>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QUrl>
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
    QMetaObject::invokeMethod(engine_, &ObsEngine::saveReplay);
}
void Controller::toggleRecording() {
    if (state_ == "Idle" || state_ == "Error")
        start();
    QMetaObject::invokeMethod(engine_, &ObsEngine::toggleRecording);
}
void Controller::screenshot() {
    auto screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;
    QDir().mkpath(settings_.clips);
    auto path = settings_.clips + "/Capture " +
                QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss-zzz") + ".png";
    if (screen->grabWindow(0).save(path))
        notify("Capture de l'écran principal sauvegardée : " + path);
    else
        notify("Capture d'écran impossible.", true);
}
bool Controller::apply(QVariantMap v) {
    if (state_ != "Idle" && state_ != "Error") {
        notify("Arrêtez la capture avant d'appliquer les réglages.", true);
        return false;
    }
    auto s = Settings::fromJson(QJsonObject::fromVariantMap(v));
    QString e;
    if (!s.save(data_ + "/settings.json", &e)) {
        notify(e, true);
        return false;
    }
    settings_ = s;
    emit configChanged();
    e = hotkeys_.configure(s.saveKey, s.recordKey, s.screenshotKey);
    if (!e.isEmpty())
        notify(e, true);
    else
        notify("Réglages enregistrés.");
    emit changed();
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
