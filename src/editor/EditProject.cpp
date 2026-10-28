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
    return {{"version", 2},          {"source", source},         {"segments", a},
            {"volume", volume},      {"hasAudio", hasAudio},     {"audioTracks", audioTracks},
            {"gameVolume", gameVolume}, {"micVolume", micVolume}, {"crop", QJsonArray{cropX, cropY, cropW, cropH}}};
}
EditProject EditProject::fromJson(const QJsonObject &o) {
    EditProject p;
    p.source = o["source"].toString();
    p.volume = o["volume"].toDouble(1);
    p.hasAudio = o["hasAudio"].toBool(true);
    p.audioTracks = o["audioTracks"].toInt(1);
    p.gameVolume = o["gameVolume"].toDouble(1);
    p.micVolume = o["micVolume"].toDouble(1);
    auto crop = o["crop"].toArray();
    if (crop.size() == 4) {
        p.cropX = crop[0].toDouble();
        p.cropY = crop[1].toDouble();
        p.cropW = crop[2].toDouble(1);
        p.cropH = crop[3].toDouble(1);
    }
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
    for (double v : {volume, gameVolume, micVolume})
        if (!std::isfinite(v) || v < 0 || v > 2)
            return "Volume invalide.";
    for (double v : {cropX, cropY, cropW, cropH})
        if (!std::isfinite(v))
            return "Recadrage invalide.";
    if (cropX < 0 || cropY < 0 || cropW < 0.05 || cropH < 0.05 || cropX + cropW > 1.0001 || cropY + cropH > 1.0001)
        return "Recadrage invalide : gardez au moins 5 % de l'image.";
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
