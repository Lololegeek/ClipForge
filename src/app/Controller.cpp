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
Controller::Controller(QString runtime, QString data, QString tools, bool test)
    : data_(data), settings_(Settings::load(data + "/settings.json")), library_(data + "/library.sqlite"),
      jobs_(tools, data + "/thumbnails"), engine_(new ObsEngine(runtime, data + "/obs")), test_(test) {
    qRegisterMetaType<Settings>();
    if (test_) {
        settings_.clips = data_ + "/clips";
        settings_.width = 1280;
        settings_.height = 720;
        settings_.fps = 30;
        settings_.bitrate = 5000;
        settings_.replaySeconds = 15;
        settings_.microphone = "disabled";
        settings_.audio = "system";
    }
    engine_->moveToThread(&worker_);
    connect(&worker_, &QThread::started, engine_, &ObsEngine::initialize);
    connect(&worker_, &QThread::finished, engine_, &QObject::deleteLater);
    connect(engine_, &ObsEngine::ready, this, [this](QVariantList e, QVariantList s, QVariantList m) {
        ready_ = true;
        encoders_ = e;
        targets_ = s;
        microphones_ = m;
        if (settings_.target.isEmpty())
            for (auto entry : s) {
                auto t = entry.toMap();
                if (t["mode"] == "display") {
                    settings_.target = t["value"].toString();
                    break;
                }
            }
        emit changed();
        emit catalogChanged();
        emit configChanged();
        if (test_)
            runSelfTest();
    });
    connect(engine_, &ObsEngine::state, this, [this](QString s) {
        state_ = s;
        emit changed();
    });
    connect(engine_, &ObsEngine::method, this, [this](QString m, QString e) {
        method_ = m;
        encoder_ = e;
        emit changed();
    });
    connect(engine_, &ObsEngine::recording, this, [this](bool r) {
        recording_ = r;
        emit changed();
    });
    connect(engine_, &ObsEngine::metrics, this, [this](QVariantMap m) {
        for (auto it = m.begin(); it != m.end(); ++it)
            metrics_[it.key()] = it.value();
        QStorageInfo disk(settings_.clips);
        metrics_["diskGB"] = std::max(0.0, double(disk.bytesAvailable()) / 1073741824.0);
        if (test_)
            testSamples_.append(QJsonObject::fromVariantMap(metrics_));
        emit changed();
    });
    connect(engine_, &ObsEngine::failure, this, [this](QString s) { notify(s, true); });
    connect(&jobs_, &MediaJobs::failure, this, [this](QString s) { notify(s, true); });
    connect(&library_, &ClipModel::failure, this, [this](QString s) { notify(s, true); });
    connect(engine_, &ObsEngine::saved, this, [this](QString p) {
        lastClip_ = p;
        jobs_.import(p, game_);
        notify("Clip sauvegardé : " + QFileInfo(p).fileName());
    });
    connect(&jobs_, &MediaJobs::imported, &library_, &ClipModel::upsert);
    connect(&jobs_, &MediaJobs::thumbnail, &library_, &ClipModel::setThumbnail);
    connect(&jobs_, &MediaJobs::exportProgress, this, [this](double p) {
        progress_ = p;
        emit changed();
    });
    connect(&jobs_, &MediaJobs::exportBusy, this, [this](bool) { emit changed(); });
    connect(&jobs_, &MediaJobs::exported, this, [this](QString p) {
        jobs_.import(p, game_);
        notify("Export terminé : " + QFileInfo(p).fileName());
    });
    if (!test_ && !QCoreApplication::arguments().contains("--ui-test") && !QCoreApplication::arguments().contains("--editor-test") &&
        !QCoreApplication::arguments().contains("--screenshot")) {
        connectTray();
        connect(&hotkeys_, &GlobalHotkeys::save, this, &Controller::saveReplay);
        connect(&hotkeys_, &GlobalHotkeys::record, this, &Controller::toggleRecording);
        connect(&hotkeys_, &GlobalHotkeys::screenshot, this, &Controller::screenshot);
        auto error = hotkeys_.configure(settings_.saveKey, settings_.recordKey, settings_.screenshotKey);
        if (!error.isEmpty())
            notify(error, true);
        detection_.setInterval(2500);
        connect(&detection_, &QTimer::timeout, this, &Controller::detect);
        detection_.start();
        QTimer::singleShot(0, this, &Controller::scan);
    }
    worker_.setObjectName("Capture OBS");
    worker_.start();
    if (!library_.isOpen())
        notify("Bibliothèque inaccessible : " + library_.error(), true);
}
Controller::~Controller() {
    QMetaObject::invokeMethod(engine_, &ObsEngine::shutdown, Qt::BlockingQueuedConnection);
    worker_.quit();
    worker_.wait();
    delete tray_;
}
void Controller::connectTray() {
    tray_ = new QSystemTrayIcon(this);
    tray_->setIcon(QApplication::windowIcon());
    tray_->setToolTip("ClipForge · F8 pour sauvegarder");
    auto menu = new QMenu;
    menu->addAction("Ouvrir ClipForge", this, &Controller::showWindow);
    menu->addAction("Sauvegarder le replay", this, &Controller::saveReplay);
    menu->addAction("Activer la capture", this, &Controller::start);
    menu->addAction("Arrêter la capture", this, &Controller::stop);
    menu->addSeparator();
    menu->addAction("Quitter", this, &Controller::quit);
    tray_->setContextMenu(menu);
    connect(tray_, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason r) {
        if (r == QSystemTrayIcon::DoubleClick)
            emit showWindow();
    });
    connect(tray_, &QObject::destroyed, menu, &QObject::deleteLater);
    tray_->show();
}
void Controller::notify(QString s, bool error) {
    message_ = s;
    qInfo().noquote() << (error ? "ERROR: " : "INFO: ") + s;
    if (tray_)
        tray_->showMessage("ClipForge", s, error ? QSystemTrayIcon::Warning : QSystemTrayIcon::Information,
                           3500);
    emit changed();
}
} // namespace cf
