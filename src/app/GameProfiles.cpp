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
void Controller::saveProfile(QString exe, QString name) {
    if (exe.isEmpty())
        return;
    auto o = settings_.json();
    o.remove("profiles");
    o["name"] = name;
    settings_.profiles[exe.toLower()] = o;
    QString e;
    if (!settings_.save(data_ + "/settings.json", &e))
        notify(e, true);
    else
        notify("Profil enregistré : " + name);
    emit configChanged();
    emit changed();
}
void Controller::removeProfile(QString exe) {
    settings_.profiles.remove(exe.toLower());
    settings_.save(data_ + "/settings.json");
    emit configChanged();
    emit changed();
}
void Controller::detect() {
    if (!settings_.autoCapture || !ready_)
        return;
    const auto exe = foregroundExecutable().toLower();
    auto profile = settings_.profiles.value(exe).toObject();
    if (profile.isEmpty()) {
        if (autoStarted_) {
            bool running = false;
            for (auto w : windows())
                if (settings_.profiles.contains(w.toMap()["executable"].toString().toLower()))
                    running = true;
            if (!running) {
                stop();
                autoStarted_ = false;
                game_.clear();
            }
        }
        return;
    }
    if (state_ == "Idle") {
        auto s = Settings::fromJson(profile);
        s.profiles = settings_.profiles;
        settings_ = s;
        emit configChanged();
        game_ = profile["name"].toString();
        autoStarted_ = true;
        start();
    }
}
void Controller::configurationFile(bool importing) {
    auto path = importing
                    ? QFileDialog::getOpenFileName(nullptr, "Importer la configuration", {}, "JSON (*.json)")
                    : QFileDialog::getSaveFileName(nullptr, "Exporter la configuration", {}, "JSON (*.json)");
    if (path.isEmpty())
        return;
    if (importing) {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) {
            notify(f.errorString(), true);
            return;
        }
        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(f.readAll(), &error);
        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            notify("Configuration JSON invalide.", true);
            return;
        }
        apply(doc.object().toVariantMap());
    } else {
        QString e;
        if (!settings_.save(path, &e))
            notify(e, true);
        else
            notify("Configuration exportée.");
    }
}
void Controller::chooseClipsFolder() {
    auto dir = QFileDialog::getExistingDirectory(nullptr, "Dossier des clips", settings_.clips);
    if (!dir.isEmpty()) {
        auto c = config();
        c["clips"] = dir;
        apply(c);
    }
}
} // namespace cf