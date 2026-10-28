#pragma once
#include <QJsonObject>
#include <QVector>
namespace cf {
struct Segment {
    double in = 0, out = 0, speed = 1;
};
class EditProject {
  public:
    QString source;
    QVector<Segment> segments;
    double volume = 1;
    bool hasAudio = true;
    // ClipForge clips carry the full mix on track 0, then game-only and microphone-only tracks when separated.
    int audioTracks = 1;
    double gameVolume = 1, micVolume = 1;
    // Normalised crop rectangle on the source frame (0..1); the full frame by default.
    double cropX = 0, cropY = 0, cropW = 1, cropH = 1;
    bool cropped() const {
        return cropX > 0 || cropY > 0 || cropW < 1 || cropH < 1;
    }
    bool separateAudio() const {
        return hasAudio && audioTracks >= 3;
    }
    QJsonObject json() const;
    static EditProject fromJson(const QJsonObject &);
    QString validate() const;
    bool save(QString path) const;
    static EditProject load(QString path);
};
} // namespace cf
