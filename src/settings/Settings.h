#pragma once
#include <QJsonObject>
#include <QString>
namespace cf {
struct Settings {
    QString clips, temporary, mode = "display", target, codec = "h264", encoder = "auto", audio = "system",
                              microphone = "disabled";
    int width = 1920, height = 1080, fps = 60, replaySeconds = 60, bitrate = 16000, memoryMB = 512;
    int saveKey = 0x77;
    double systemVolume = 1.0, micVolume = 1.0;
    bool closeToTray = true, autoCapture = false, startMinimized = false, gameOverlay = true, autoUpdate = true, alwaysReplay = true;
    QJsonObject profiles;
    static Settings defaults();
    static Settings fromJson(const QJsonObject &o);
    QJsonObject json() const;
    QString validate() const;
    static Settings load(const QString &path);
    bool save(const QString &path, QString *error = nullptr) const;
};
} // namespace cf
