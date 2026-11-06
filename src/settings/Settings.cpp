#include "Settings.h"
#include "hotkeys/GlobalHotkeys.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>
#include <cmath>
namespace cf {
Settings Settings::defaults() {
    Settings s;
    s.clips = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/ClipForge";
    s.temporary = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return s;
}
QJsonObject Settings::json() const {
    QJsonObject o;
#define STR(x) o[#x] = x
    STR(clips);
    STR(temporary);
    STR(mode);
    STR(target);
    STR(codec);
    STR(encoder);
    STR(audio);
    STR(microphone);
    STR(width);
    STR(height);
    STR(fps);
    STR(replaySeconds);
    STR(bitrate);
    STR(memoryMB);
    STR(saveKey);
    STR(markerKey);
    STR(micKey);
    STR(clipSeconds);
    STR(cleanupGB);
    STR(cleanupDays);
    STR(separateTracks);
    STR(pushToTalk);
    STR(saveSound);
    STR(systemVolume);
    STR(micVolume);
    STR(closeToTray);
    STR(autoCapture);
    STR(startMinimized);
    STR(gameOverlay);
    STR(captureCursor);
    STR(autoUpdate);
    STR(alwaysReplay);
    STR(onboarded);
    STR(fileTemplate);
    STR(discordWebhook);
    STR(gameFolders);
    STR(profiles);
#undef STR
    return o;
}
Settings Settings::fromJson(const QJsonObject &o) {
    Settings s = defaults();
#define S(x)                                                                                                 \
    if (o.contains(#x))                                                                                      \
    s.x = o[#x].toString(s.x)
#define I(x)                                                                                                 \
    if (o.contains(#x))                                                                                      \
    s.x = o[#x].toInt(s.x)
#define B(x)                                                                                                 \
    if (o.contains(#x))                                                                                      \
    s.x = o[#x].toBool(s.x)
    S(clips);
    S(temporary);
    S(fileTemplate);
    S(discordWebhook);
    S(mode);
    S(target);
    S(codec);
    S(encoder);
    S(audio);
    S(microphone);
    I(width);
    I(height);
    I(fps);
    I(replaySeconds);
    I(bitrate);
    I(memoryMB);
    I(saveKey);
    I(markerKey);
    I(micKey);
    I(clipSeconds);
    I(cleanupGB);
    I(cleanupDays);
    B(separateTracks);
    B(pushToTalk);
    B(saveSound);
    B(closeToTray);
    B(autoCapture);
    B(startMinimized);
    B(gameOverlay);
    B(captureCursor);
    B(autoUpdate);
    B(alwaysReplay);
    B(onboarded);
    B(gameFolders);
    if (o.contains("profiles"))
        s.profiles = o["profiles"].toObject();
    if (o["systemVolume"].isDouble())
        s.systemVolume = o["systemVolume"].toDouble();
    if (o["micVolume"].isDouble())
        s.micVolume = o["micVolume"].toDouble();
#undef S
#undef I
#undef B
    return s;
}
QString Settings::validate() const {
    if (clips.isEmpty() || !QDir::isAbsolutePath(clips))
        return "Choisissez un dossier de clips absolu.";
    if (!native() && (width < 320 || width > 7680 || height < 180 || height > 4320 || width % 2 || height % 2))
        return "Résolution paire requise, de 320×180 à 7680×4320 (ou 0×0 pour la résolution native de l'écran).";
    if (fps != 30 && fps != 60 && fps != 120 && fps != 144)
        return "FPS autorisés : 30, 60, 120 ou 144.";
    if (replaySeconds < 5 || replaySeconds > 3600 || memoryMB < 64 || memoryMB > 8192)
        return "Tampon : 5 à 3600 s ; mémoire : 64 à 8192 Mo.";
    if (bitrate < 500 || bitrate > 200000)
        return "Débit : 500 à 200000 kbit/s.";
    if (!QStringList{"display", "window", "game"}.contains(mode) ||
        !QStringList{"h264", "hevc", "av1"}.contains(codec))
        return "Mode ou codec non pris en charge.";
    if (!QStringList{"disabled", "system", "process"}.contains(audio))
        return "Source audio invalide.";
    if (!hotkey::valid(saveKey) || hotkey::mods(saveKey) & hotkey::kShift)
        return "Raccourci de sauvegarde invalide : une touche F1–F24, ou Ctrl/Alt + une touche (Maj est réservé au replay complet).";
    const int fullKey = saveKey | (hotkey::kShift << 16);
    for (int key : {markerKey, micKey})
        if (key && (!hotkey::valid(key) || key == saveKey || key == fullKey))
            return "Raccourcis invalides : chacun doit être différent de la sauvegarde (et de Maj + sauvegarde).";
    if (markerKey && markerKey == micKey)
        return "Le marqueur et le micro doivent utiliser des touches différentes.";
    if (pushToTalk && !micKey)
        return "Le push-to-talk exige une touche micro.";
    if (clipSeconds && (clipSeconds < 5 || clipSeconds > replaySeconds))
        return "La durée du clip court doit tenir dans le tampon (5 s minimum).";
    if (cleanupGB < 0 || cleanupGB > 100000 || cleanupDays < 0 || cleanupDays > 3650)
        return "Nettoyage : 0 à 100000 Go, 0 à 3650 jours.";
    if (!std::isfinite(systemVolume) || !std::isfinite(micVolume) || systemVolume < 0 || systemVolume > 2 ||
        micVolume < 0 || micVolume > 2)
        return "Volumes invalides.";
    return {};
}
const QStringList &Settings::appKeys() {
    static const QStringList k{"clips",       "temporary",  "closeToTray", "autoCapture", "startMinimized",
                               "gameOverlay", "autoUpdate", "alwaysReplay", "onboarded",  "profiles",
                               "cleanupGB",   "cleanupDays", "saveKey",      "markerKey",  "micKey",
                               "pushToTalk",  "saveSound", "fileTemplate", "gameFolders", "discordWebhook"};
    return k;
}
Settings Settings::load(const QString &p) {
    QFile f(p);
    if (!f.open(QIODevice::ReadOnly))
        return defaults();
    auto s = fromJson(QJsonDocument::fromJson(f.readAll()).object());
    s.repair();
    return s;
}
void Settings::repair() {
    // One bad field (hand edit, older/newer version) must only reset that field, never the whole file.
    const Settings d = defaults();
    if (clips.isEmpty() || !QDir::isAbsolutePath(clips))
        clips = d.clips;
    if (!native() && (width < 320 || width > 7680 || height < 180 || height > 4320 || width % 2 || height % 2)) {
        width = d.width;
        height = d.height;
    }
    if (fps != 30 && fps != 60 && fps != 120 && fps != 144)
        fps = d.fps;
    if (replaySeconds < 5 || replaySeconds > 3600)
        replaySeconds = d.replaySeconds;
    if (memoryMB < 64 || memoryMB > 8192)
        memoryMB = d.memoryMB;
    if (bitrate < 500 || bitrate > 200000)
        bitrate = d.bitrate;
    if (!QStringList{"display", "window", "game"}.contains(mode))
        mode = d.mode;
    if (!QStringList{"h264", "hevc", "av1"}.contains(codec))
        codec = d.codec;
    if (!QStringList{"disabled", "system", "process"}.contains(audio))
        audio = d.audio;
    if (!hotkey::valid(saveKey) || hotkey::mods(saveKey) & hotkey::kShift)
        saveKey = d.saveKey;
    for (int *key : {&markerKey, &micKey})
        if (*key && (!hotkey::valid(*key) || *key == saveKey || *key == (saveKey | (hotkey::kShift << 16))))
            *key = 0;
    if (markerKey && markerKey == micKey)
        micKey = 0;
    if (pushToTalk && !micKey)
        pushToTalk = false;
    if (clipSeconds && (clipSeconds < 5 || clipSeconds > replaySeconds))
        clipSeconds = 0;
    if (cleanupGB < 0 || cleanupGB > 100000)
        cleanupGB = 0;
    if (cleanupDays < 0 || cleanupDays > 3650)
        cleanupDays = 0;
    if (!std::isfinite(systemVolume) || systemVolume < 0 || systemVolume > 2)
        systemVolume = d.systemVolume;
    if (!std::isfinite(micVolume) || micVolume < 0 || micVolume > 2)
        micVolume = d.micVolume;
    if (!validate().isEmpty()) {
        auto keep = profiles;
        const bool seen = onboarded;
        *this = d;
        profiles = keep;
        onboarded = seen;
    }
}
bool Settings::save(const QString &p, QString *error) const {
    auto e = validate();
    if (!e.isEmpty()) {
        if (error)
            *error = e;
        return false;
    }
    QDir().mkpath(QFileInfo(p).absolutePath());
    QSaveFile f(p);
    if (!f.open(QIODevice::WriteOnly) || f.write(QJsonDocument(json()).toJson()) < 0 || !f.commit()) {
        if (error)
            *error = f.errorString();
        return false;
    }
    return true;
}
} // namespace cf
