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
void MediaJobs::import(QString p, QString game, bool priority) {
    p = QFileInfo(p).absoluteFilePath();
    if (pending_.contains(p))
        return;
    if (queue_.size() >= 256) {
        emit failure("File d'importation pleine. Importez les fichiers par petits groupes.");
        return;
    }
    pending_.insert(p);
    // A clip the user just saved jumps ahead of a library scan so its thumbnail shows up immediately.
    if (priority)
        queue_.prepend({p, game});
    else
        queue_.enqueue({p, game});
    next();
}
void MediaJobs::next() {
    // Probing and thumbnailing are light, single-threaded processes: a few in parallel keep the library
    // filling quickly without competing with a game for the CPU.
    while (active_ < kParallelJobs && !queue_.isEmpty())
        run(queue_.dequeue());
}
void MediaJobs::run(Import job) {
    auto p = new QProcess(this);
    ++active_;
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
            --active_;
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
                    --active_;
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
                if (QFileInfo(image).size() > 0) {
                    // Same path, size and modification time: the cached thumbnail is still valid.
                    emit thumbnail(job.path, image);
                    pending_.remove(job.path);
                    --active_;
                    next();
                    return;
                }
                auto thumb = new QProcess(this);
                connect(thumb, &QProcess::errorOccurred, this, [this, thumb, job](QProcess::ProcessError e) {
                    if (e == QProcess::FailedToStart) {
                        pending_.remove(job.path);
                        --active_;
                        thumb->deleteLater();
                        next();
                    }
                });
                connect(thumb, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
                        [this, thumb, job, image](int c, QProcess::ExitStatus) {
                            if (c == 0 && QFileInfo::exists(image))
                                emit thumbnail(job.path, image);
                            pending_.remove(job.path);
                            --active_;
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
                              "-an", "-sn", "-vf", "scale=480:-2:flags=fast_bilinear", "-q:v", "4", "-threads", "1", image});
            });
    p->start();
}
double MediaJobs::outputDuration(const EditProject &p) {
    double total = 0;
    for (auto s : p.segments)
        total += (s.out - s.in) / s.speed;
    return total;
}
QStringList MediaJobs::exportArguments(const EditProject &p, QString dest, const ExportOptions &o) {
    QStringList a{"-v", "error", "-nostdin", "-y", "-i", p.source};
    const bool animated = o.format == "gif" || o.format == "webp";
    const bool audio = p.hasAudio && !animated;
    int height = o.height, kbps = 0;
    if (o.maxMB > 0 && !animated) {
        // Single pass under a size cap: 4 % container margin, 128 kbit/s for audio.
        const double seconds = std::max(0.5, outputDuration(p));
        kbps = std::max(150, int(o.maxMB * 8192 * 0.96 / seconds) - (audio ? 128 : 0));
        if (kbps < 1500 && (height == 0 || height > 480))
            height = 480;
        else if (kbps < 3500 && (height == 0 || height > 720))
            height = 720;
    }
    // Crop, then the optional 9:16 centre-crop, then scaling: applied once on the concatenated stream.
    QStringList post;
    if (p.cropped())
        post << QString("crop=trunc(iw*%3/2)*2:trunc(ih*%4/2)*2:iw*%1:ih*%2")
                    .arg(p.cropX, 0, 'f', 6)
                    .arg(p.cropY, 0, 'f', 6)
                    .arg(p.cropW, 0, 'f', 6)
                    .arg(p.cropH, 0, 'f', 6);
    if (o.vertical)
        post << "crop=trunc(min(iw\\,ih*9/16)/2)*2:trunc(min(ih\\,iw*16/9)/2)*2";
    if (height > 0)
        post << QString("scale=-2:%1:flags=lanczos").arg(height);
    QStringList filters;
    QString joins;
    for (qsizetype i = 0; i < p.segments.size(); ++i) {
        auto s = p.segments[i];
        QString idx = QString::number(i), speed = QString::number(s.speed, 'g', 8);
        const QString range = QString("start=%1:end=%2").arg(s.in, 0, 'f', 6).arg(s.out, 0, 'f', 6);
        filters << QString("[0:v]trim=%1,setpts=(PTS-STARTPTS)/%2,fps=%3,setsar=1[v%4]")
                       .arg(range)
                       .arg(speed)
                       .arg(o.fps)
                       .arg(idx);
        joins += "[v" + idx + "]";
        if (audio) {
            QString tempo;
            if (s.speed < 0.5)
                tempo = "atempo=0.5,atempo=" + QString::number(s.speed / 0.5);
            else if (s.speed > 2)
                tempo = "atempo=2,atempo=" + QString::number(s.speed / 2);
            else
                tempo = "atempo=" + speed;
            if (p.separateAudio()) {
                // Rebuild the mix from the game and microphone tracks so each keeps its own level.
                filters << QString("[0:a:1]atrim=%1,asetpts=PTS-STARTPTS,volume=%2[ag%3]")
                               .arg(range)
                               .arg(p.gameVolume)
                               .arg(idx)
                        << QString("[0:a:2]atrim=%1,asetpts=PTS-STARTPTS,volume=%2[am%3]")
                               .arg(range)
                               .arg(p.micVolume)
                               .arg(idx)
                        << QString("[ag%1][am%1]amix=inputs=2:duration=longest:normalize=0,%2,volume=%3[a%1]")
                               .arg(idx)
                               .arg(tempo)
                               .arg(p.volume);
            } else {
                filters << QString("[0:a:0]atrim=%1,asetpts=PTS-STARTPTS,%2,volume=%3[a%4]")
                               .arg(range)
                               .arg(tempo)
                               .arg(p.volume)
                               .arg(idx);
            }
            joins += "[a" + idx + "]";
        }
    }
    const QString tail = post.isEmpty() ? QString() : "," + post.join(',');
    filters << joins + QString("concat=n=%1:v=1:a=%2[c]").arg(p.segments.size()).arg(audio ? 1 : 0) +
                   (audio ? "[a]" : "");
    if (o.format == "gif")
        filters << "[c]null" + tail +
                       ",split[g1][g2];[g1]palettegen=stats_mode=diff[pal];[g2][pal]paletteuse=dither=bayer:bayer_scale=4[v]";
    else
        filters << "[c]null" + tail + "[v]";
    a << "-filter_complex" << filters.join(';') << "-map" << "[v]";
    if (o.format == "gif") {
        a << "-loop" << "0";
    } else if (o.format == "webp") {
        a << "-c:v" << "libwebp_anim" << "-quality" << "70" << "-compression_level" << "4" << "-loop" << "0";
    } else {
        if (audio)
            a << "-map" << "[a]" << "-c:a" << "aac" << "-b:a" << (kbps ? "128k" : "192k");
        a << "-c:v" << (o.codec == "hevc" ? "libx265" : "libx264") << "-preset" << "fast";
        if (kbps)
            a << "-b:v" << QString("%1k").arg(kbps) << "-maxrate" << QString("%1k").arg(kbps * 3 / 2) << "-bufsize"
              << QString("%1k").arg(kbps * 2);
        else
            a << "-crf" << QString::number(o.crf);
        a << "-pix_fmt" << "yuv420p" << "-threads" << "4" << "-movflags" << "+faststart";
        if (o.codec == "hevc")
            a << "-x265-params" << "pools=4:frame-threads=2";
    }
    a << "-progress" << "pipe:1" << dest;
    return a;
}
bool MediaJobs::exportVideo(EditProject project, QString dest, ExportOptions o) {
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
    if (!QStringList{"h264", "hevc"}.contains(o.codec) || !QStringList{"mp4", "gif", "webp"}.contains(o.format) ||
        o.fps < 1 || o.fps > 144 || o.height < 0 || o.height > 4320 || o.maxMB < 0 || o.crf < 0 || o.crf > 51) {
        emit failure("Paramètres d'export invalides.");
        return false;
    }
    exportTemp_ = QFileInfo(dest).absolutePath() + "/.clipforge-" + QUuid::createUuid().toString(QUuid::Id128) +
                  "." + o.format;
    auto p = new QProcess(this);
    exporter_ = p;
    emit exportBusy(true);
    const double total = outputDuration(project);
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
    p->start(tool("ffmpeg"), exportArguments(project, exportTemp_, o));
    return true;
}
void MediaJobs::trimTail(QString path, double seconds) {
    const QFileInfo f(path);
    const QString temp = f.absolutePath() + "/.clipforge-" + QUuid::createUuid().toString(QUuid::Id128) + ".mp4";
    auto p = new QProcess(this);
    auto done = [this, p, path, temp](bool ok) {
        // The full replay stays untouched whenever the shortened copy cannot replace it.
        const bool replaced = ok && QFileInfo(temp).size() > 0 && QFile::remove(path) && QFile::rename(temp, path);
        if (!replaced) {
            QFile::remove(temp);
            emit failure("Raccourcissement impossible : le clip complet est conservé.");
        }
        emit trimmed(path);
        p->deleteLater();
    };
    connect(p, &QProcess::errorOccurred, this, [done](QProcess::ProcessError e) {
        if (e == QProcess::FailedToStart)
            done(false);
    });
    connect(p, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [done](int code, QProcess::ExitStatus status) { done(code == 0 && status == QProcess::NormalExit); });
    p->start(tool("ffmpeg"), {"-v", "error", "-nostdin", "-y", "-sseof", QString::number(-seconds, 'f', 3), "-i",
                              path, "-map", "0", "-c", "copy", "-movflags", "+faststart", temp});
}
void MediaJobs::snapshot(QString source, double seconds, QString dest) {
    QDir().mkpath(QFileInfo(dest).absolutePath());
    auto p = new QProcess(this);
    connect(p, &QProcess::errorOccurred, this, [this, p](QProcess::ProcessError e) {
        if (e == QProcess::FailedToStart) {
            emit failure("FFmpeg introuvable.");
            p->deleteLater();
        }
    });
    connect(p, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, p, dest](int code, QProcess::ExitStatus) {
                if (code == 0 && QFileInfo(dest).size() > 0)
                    emit snapshotReady(dest);
                else
                    emit failure("Capture d'image impossible : " +
                                 QString::fromUtf8(p->readAllStandardError()).right(400));
                p->deleteLater();
            });
    // Seeking after -i decodes up to the exact frame, which matters after frame-by-frame stepping.
    p->start(tool("ffmpeg"), {"-v", "error", "-nostdin", "-y", "-i", source, "-ss", QString::number(seconds, 'f', 3),
                              "-frames:v", "1", dest});
}

} // namespace cf
