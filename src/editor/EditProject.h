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
    QJsonObject json() const;
    static EditProject fromJson(const QJsonObject &);
    QString validate() const;
    bool save(QString path) const;
    static EditProject load(QString path);
};
} // namespace cf
