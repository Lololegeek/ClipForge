#include "Controller.h"
#include "Overlay.h"
#include "hotkeys/GlobalHotkeys.h"
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
// A profile only overrides capture fields; app options, hotkeys and the clips folder stay global.
Settings Controller::withProfile(const Settings &base, const QJsonObject &profile) {
    auto merged = base.json();
    for (auto it = profile.begin(); it != profile.end(); ++it)
        if (it.key() != "name" && !Settings::appKeys().contains(it.key()))
            merged[it.key()] = it.value();
    auto s = Settings::fromJson(merged);
    s.repair();
    return s;
}
void Controller::saveProfile(QString exe, QString name, QVariantMap overrides) {
    if (exe.isEmpty())
        return;
    auto &base = stored();
    auto o = base.json();
    for (const auto &k : Settings::appKeys())
        o.remove(k);
    o["name"] = name;
    // Values picked in the game setup window win over the global settings (only these fields can differ per game).
    static const QStringList allowed{"width", "height", "fps", "replaySeconds", "bitrate"};
    for (auto it = overrides.cbegin(); it != overrides.cend(); ++it)
        if (allowed.contains(it.key()))
            o[it.key()] = QJsonValue::fromVariant(it.value());
    base.profiles[exe.toLower()] = o;
    if (!base.autoCapture) { // a profile does nothing unless automatic capture is on
        base.autoCapture = true;
        settings_.autoCapture = true;
        global_.autoCapture = true;
    }
    settings_.profiles = base.profiles;
    QString e;
    if (!base.save(data_ + "/settings.json", &e))
        notify(e, true);
    else
        notify("Profil enregistré : " + name);
    emit configChanged();
    emit changed();
}
void Controller::removeProfile(QString exe) {
    auto &base = stored();
    base.profiles.remove(exe.toLower());
    settings_.profiles = base.profiles;
    base.save(data_ + "/settings.json");
    emit configChanged();
    emit changed();
}
void Controller::detect() {
    if (!stored().autoCapture || !ready_)
        return;
    const bool idle = state_ == "Idle" || state_ == "Error";
    // Never switch settings in the middle of a start, a save or a manual recording: retry on the next tick.
    if (!idle && state_ != "Buffering")
        return;
    const auto exe = foregroundExecutable().toLower();
    const auto profile = stored().profiles.value(exe).toObject();
    if (profile.isEmpty()) {
        if (profileExe_.isEmpty() && !autoStarted_)
            return;
        bool running = false;
        for (auto w : windows()) {
            const auto running_exe = w.toMap()["executable"].toString().toLower();
            if (profileExe_.isEmpty() ? stored().profiles.contains(running_exe)
                                      : running_exe == profileExe_ && stored().profiles.contains(profileExe_))
                running = true;
        }
        if (running)
            return;
        const bool stopReplay = autoStarted_ && !stored().alwaysReplay;
        if (!profileExe_.isEmpty()) {
            const Settings base = global_;
            profileExe_.clear();
            if (stopReplay)
                settings_ = base;
            else
                useSettings(base, "Jeu fermé : retour à vos réglages, redémarrage du replay…");
        }
        if (stopReplay)
            stop();
        autoStarted_ = false;
        game_.clear();
        configureHotkeys();
        emit changed();
        return;
    }
    if (exe == profileExe_)
        return;
    if (profileExe_.isEmpty())
        global_ = settings_;
    profileExe_ = exe;
    game_ = profile["name"].toString();
    useSettings(withProfile(global_, profile), "Profil « " + game_ + " » : redémarrage du replay…");
    if (idle) {
        autoStarted_ = true;
        start();
    }
    // Reminder of the shortcuts for a few seconds once the game is up (after the "restarting" toast).
    const QString game = game_;
    QTimer::singleShot(1800, this, [this, game] {
        if (game_ != game)
            return;
        const auto &s = settings_;
        const QString nl(QChar(10));
        QString keys = "Clip : " + hotkey::name(s.saveKey) + "   ·   Tout le replay : " +
                       hotkey::name(s.saveKey | (hotkey::kShift << 16));
        if (s.markerKey)
            keys += nl + "Marqueur : " + hotkey::name(s.markerKey);
        if (s.micKey)
            keys += (s.markerKey ? QString("   ·   ") : nl) + "Micro : " + hotkey::name(s.micKey);
        toast("ClipForge prêt · " + game, keys + nl + "Replay de " + QString::number(s.replaySeconds) + " s en mémoire",
              Overlay::Success);
    });
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
        if (!stored().save(path, &e))
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