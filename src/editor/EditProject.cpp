#include "EditProject.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <cmath>
namespace cf {
QJsonObject EditProject::json() const {
    QJsonArray a;
    for (auto &s : segments)
        a << QJsonObject{{"in", s.in}, {"out", s.out}, {"speed", s.speed}};
    return {{"version", 1}, {"source", source}, {"segments", a}, {"volume", volume}, {"hasAudio", hasAudio}};
}
EditProject EditProject::fromJson(const QJsonObject &o) {
    EditProject p;
    p.source = o["source"].toString();
    p.volume = o["volume"].toDouble(1);
    p.hasAudio = o["hasAudio"].toBool(true);
    for (auto v : o["segments"].toArray()) {
        auto s = v.toObject();
        p.segments << Segment{s["in"].toDouble(), s["out"].toDouble(), s["speed"].toDouble(1)};
    }
    return p;
}
QString EditProject::validate() const {
    if (source.isEmpty() || !QFileInfo::exists(source))
        return "Vidéo source introuvable.";
    if (segments.isEmpty() || segments.size() > 128)
        return "Ajoutez entre 1 et 128 passages.";
    if (!std::isfinite(volume) || volume < 0 || volume > 2)
        return "Volume invalide.";
    for (auto s : segments)
        if (!std::isfinite(s.in) || !std::isfinite(s.out) || !std::isfinite(s.speed) || s.in < 0 ||
            s.out <= s.in || s.speed < 0.25 || s.speed > 4)
            return "Passage invalide : vérifiez les points d'entrée, de sortie et la vitesse.";
    return {};
}
bool EditProject::save(QString p) const {
    QSaveFile f(p);
    return f.open(QIODevice::WriteOnly) && f.write(QJsonDocument(json()).toJson()) > 0 && f.commit();
}
EditProject EditProject::load(QString p) {
    QFile f(p);
    if (!f.open(QIODevice::ReadOnly))
        return {};
    return fromJson(QJsonDocument::fromJson(f.readAll()).object());
}
} // namespace cf
