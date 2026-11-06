#pragma once
#include <QJsonObject>
#include <QString>
#include <QStringList>
namespace cf {
struct Settings {
    QString clips, temporary, mode = "display", target, codec = "h264", encoder = "auto", audio = "system",
                              microphone = "disabled";
    int width = 1920, height = 1080, fps = 60, replaySeconds = 60, bitrate = 16000, memoryMB = 512;
    int saveKey = 0x77;
    // Optional F-keys (0 = off). Shift+saveKey always keeps the whole buffer; saveKey keeps clipSeconds (0 = all).
    int markerKey = 0, micKey = 0, clipSeconds = 0;
    // Library cleanup limits (0 = off). Favourites are never removed.
    int cleanupGB = 0, cleanupDays = 0;
    double systemVolume = 1.0, micVolume = 1.0;
    bool closeToTray = true, autoCapture = false, startMinimized = false, gameOverlay = true, captureCursor = true, autoUpdate = true, alwaysReplay = true, onboarded = false;
    bool separateTracks = true, pushToTalk = false, saveSound = true;
    QString fileTemplate; // empty = keep OBS's own name; placeholders {jeu} {date} {heure}
    QString discordWebhook;
    bool gameFolders = false;
    QJsonObject profiles;
    static Settings defaults();
    static Settings fromJson(const QJsonObject &o);
    QJsonObject json() const;
    QString validate() const;
    // 0×0 means "follow the screen resolution"; Controller::start resolves it to real pixels.
    bool native() const { return width == 0 && height == 0; }
    void repair(); // resets only the invalid fields to their defaults
    // Fields that are about the app, not the capture: a game profile never overrides them.
    static const QStringList &appKeys();
    static Settings load(const QString &path);
    bool save(const QString &path, QString *error = nullptr) const;
};
} // namespace cf
