#include "MediaJobs.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTimer>
#include <QUuid>
namespace cf {
MediaJobs::MediaJobs(QString t, QString c, QObject *p)
    : QObject(p), tools_(std::move(t)), cache_(std::move(c)) {
    QDir().mkpath(cache_);
}
MediaJobs::~MediaJobs() {
    for (auto p : findChildren<QProcess *>()) {
        if (p->state() != QProcess::NotRunning) {
            p->kill();
            p->waitForFinished(3000);
        }
    }
    if (!exportTemp_.isEmpty())
        QFile::remove(exportTemp_);
}
QString MediaJobs::tool(QString n) const {
    auto p = tools_ + "/" + n + ".exe";
    return QFileInfo::exists(p) ? p : QStandardPaths::findExecutable(n);
}
void MediaJobs::import(QString p, QString game) {
    p = QFileInfo(p).absoluteFilePath();
    if (pending_.contains(p))
        return;
    if (queue_.size() >= 256) {
        emit failure("File d'importation pleine. Importez les fichiers par petits groupes.");
        return;
    }
    pending_.insert(p);
    queue_.enqueue({p, game});
    next();
}
void MediaJobs::next() {
    if (probe_ || queue_.isEmpty())
        return;
    auto job = queue_.dequeue();
    auto p = new QProcess(this);
    probe_ = p;
    p->setProgram(tool("ffprobe"));
    p->setArguments({"-v", "error", "-show_format", "-show_streams", "-of", "json", job.path});
    auto timeout = new QTimer(p);
    timeout->setSingleShot(true);
    timeout->setInterval(20000);
    connect(timeout, &QTimer::timeout, p, &QProcess::kill);
    timeout->start();
    connect(p, &QProcess::errorOccurred, this, [this, p, job](QProcess::ProcessError e) {
        if (e == QProcess::FailedToStart) {
            emit failure("FFprobe introuvable. Vérifiez la distribution.");
            pending_.remove(job.path);
            probe_ = nullptr;
            p->deleteLater();
            next();
        }
    });
    connect(p, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, p, job](int code, QProcess::ExitStatus status) {
                auto root = QJsonDocument::fromJson(p->readAllStandardOutput()).object();
                auto format = root["format"].toObject();
                bool video = false;
                for (auto stream : root["streams"].toArray())
                    if (stream.toObject()["codec_type"] == "video")
                        video = true;
                if (code || status != QProcess::NormalExit || !video) {
                    emit failure("Vidéo invalide ou inaccessible : " + QFileInfo(job.path).fileName());
                    pending_.remove(job.path);
                    probe_ = nullptr;
                    p->deleteLater();
                    next();
                    return;
                }
                QFileInfo f(job.path);
                double duration = format["duration"].toString().toDouble();
                emit imported({{"path", job.path},
                               {"name", f.completeBaseName()},
                               {"size", f.size()},
                               {"duration", duration},
                               {"created", f.lastModified().toString(Qt::ISODateWithMs)},
                               {"game", job.game}});
                auto hash = QCryptographicHash::hash((job.path + QString::number(f.size()) +
                                                      QString::number(f.lastModified().toMSecsSinceEpoch()))
                                                         .toUtf8(),
                                                     QCryptographicHash::Sha256)
                                .toHex();
                QString image = cache_ + "/" + hash + ".jpg";
                p->deleteLater();
                auto thumb = new QProcess(this);
                probe_ = thumb;
                connect(thumb, &QProcess::errorOccurred, this, [this, thumb, job](QProcess::ProcessError e) {
                    if (e == QProcess::FailedToStart) {
                        pending_.remove(job.path);
                        probe_ = nullptr;
                        thumb->deleteLater();
                        next();
                    }
                });
                connect(thumb, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
                        [this, thumb, job, image](int c, QProcess::ExitStatus) {
                            if (c == 0 && QFileInfo::exists(image))
                                emit thumbnail(job.path, image);
                            pending_.remove(job.path);
                            probe_ = nullptr;
                            thumb->deleteLater();
                            next();
                        });
                QTimer::singleShot(20000, thumb, [thumb] {
                    if (thumb->state() != QProcess::NotRunning)
                        thumb->kill();
                });
                thumb->start(tool("ffmpeg"),
                             {"-v", "error", "-nostdin", "-y", "-ss",
                              QString::number(std::min(1.0, duration / 2)), "-i", job.path, "-frames:v", "1",
                              "-vf", "scale=480:-2", "-threads", "1", image});
            });
    p->start();
}
QStringList MediaJobs::exportArguments(const EditProject &p, QString dest, int height, int fps,
                                       QString codec) {
    QStringList a{"-v", "error", "-nostdin", "-y", "-i", p.source};
    QStringList filters;
    QString joins;
    for (qsizetype i = 0; i < p.segments.size(); ++i) {
        auto s = p.segments[i];
        QString idx = QString::number(i), speed = QString::number(s.speed, 'g', 8);
        QString vf = QString("[0:v]trim=start=%1:end=%2,setpts=(PTS-STARTPTS)/%3")
                         .arg(s.in, 0, 'f', 6)
                         .arg(s.out, 0, 'f', 6)
                         .arg(speed);
        if (height > 0)
            vf += QString(",scale=-2:%1").arg(height);
        vf += QString(",fps=%1,setsar=1[v%2]").arg(fps).arg(idx);
        filters << vf;
        joins += "[v" + idx + "]";
        if (p.hasAudio) {
            QString tempo;
            if (s.speed < 0.5)
                tempo = "atempo=0.5,atempo=" + QString::number(s.speed / 0.5);
            else if (s.speed > 2)
                tempo = "atempo=2,atempo=" + QString::number(s.speed / 2);
            else
                tempo = "atempo=" + speed;
            filters << QString("[0:a:0]atrim=start=%1:end=%2,asetpts=PTS-STARTPTS,%3,volume=%4[a%5]")
                           .arg(s.in, 0, 'f', 6)
                           .arg(s.out, 0, 'f', 6)
                           .arg(tempo)
                           .arg(p.volume)
                           .arg(idx);
            joins += "[a" + idx + "]";
        }
    }
    filters << joins + QString("concat=n=%1:v=1:a=%2[v]").arg(p.segments.size()).arg(p.hasAudio ? 1 : 0) +
                   (p.hasAudio ? "[a]" : "");
    a << "-filter_complex" << filters.join(';') << "-map" << "[v]";
    if (p.hasAudio)
        a << "-map" << "[a]" << "-c:a" << "aac" << "-b:a" << "192k";
    a << "-c:v" << (codec == "hevc" ? "libx265" : "libx264") << "-preset" << "fast" << "-crf" << "20"
      << "-pix_fmt" << "yuv420p" << "-threads" << "4" << "-movflags" << "+faststart" << "-progress"
      << "pipe:1" << dest;
    if (codec == "hevc") {
        a.insert(a.size() - 1, "-x265-params");
        a.insert(a.size() - 1, "pools=4:frame-threads=2");
    }
    return a;
}
bool MediaJobs::exportVideo(EditProject project, QString dest, int height, int fps, QString codec) {
    auto e = project.validate();
    if (!e.isEmpty()) {
        emit failure(e);
        return false;
    }
    if (exporter_) {
        emit failure("Un export est déjà en cours.");
        return false;
    }
    if (QFileInfo(dest).absoluteFilePath() == QFileInfo(project.source).absoluteFilePath() ||
        QFileInfo::exists(dest)) {
        emit failure("Choisissez un nouveau fichier de sortie pour préserver les vidéos existantes.");
        return false;
    }
    if (!QStringList{"h264", "hevc"}.contains(codec) || fps < 1 || fps > 144 || height < 0 || height > 4320) {
        emit failure("Paramètres d'export invalides.");
        return false;
    }
    exportTemp_ =
        QFileInfo(dest).absolutePath() + "/.clipforge-" + QUuid::createUuid().toString(QUuid::Id128) + ".mp4";
    auto p = new QProcess(this);
    exporter_ = p;
    emit exportBusy(true);
    double total = 0;
    for (auto s : project.segments)
        total += (s.out - s.in) / s.speed;
    connect(p, &QProcess::readyReadStandardOutput, this, [this, p, total] {
        for (auto line : p->readAllStandardOutput().split('\n'))
            if (line.startsWith("out_time_us="))
                emit exportProgress(std::clamp(line.mid(12).toDouble() / 1000000.0 / total, 0.0, 1.0));
    });
    connect(p, &QProcess::errorOccurred, this, [this, p](QProcess::ProcessError e) {
        if (e == QProcess::FailedToStart) {
            emit failure("FFmpeg introuvable.");
            QFile::remove(exportTemp_);
            exportTemp_.clear();
            exporter_ = nullptr;
            emit exportBusy(false);
            p->deleteLater();
        }
    });
    connect(p, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, p, dest](int code, QProcess::ExitStatus status) {
                bool ok = code == 0 && status == QProcess::NormalExit && QFileInfo(exportTemp_).size() > 0 &&
                          QFile::rename(exportTemp_, dest);
                if (ok) {
                    emit exportProgress(1);
                    emit exported(dest);
                } else {
                    emit failure("Export impossible : " +
                                 QString::fromUtf8(p->readAllStandardError()).right(1600));
                    QFile::remove(exportTemp_);
                }
                exportTemp_.clear();
                exporter_ = nullptr;
                emit exportBusy(false);
                p->deleteLater();
            });
    p->start(tool("ffmpeg"), exportArguments(project, exportTemp_, height, fps, codec));
    return true;
}
} // namespace cf
