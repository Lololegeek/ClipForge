#include "SlideshowExport.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QStandardPaths>
#include <QUuid>
namespace cf {
namespace {
// Fades a title in, keeps it about three seconds, then fades it out.
const QString kTitleAlpha = "'if(lt(t,0.4),t/0.4,if(lt(t,3.2),1,if(lt(t,3.8),(3.8-t)/0.6,0)))'";
const QString kFrame = "scale=1920:1080:force_original_aspect_ratio=decrease,pad=1920:1080:(ow-iw)/2:(oh-ih)/2:black,"
                       "setsar=1,fps=30,format=yuv420p";
QStringList encoderArgs() {
    return {"-c:v", "libx264", "-preset", "veryfast", "-crf", "21", "-pix_fmt", "yuv420p", "-video_track_timescale", "15360",
            "-c:a", "aac", "-b:a", "160k", "-ar", "48000", "-ac", "2"};
}
} // namespace

SlideshowExport::SlideshowExport(QString tools, QObject *parent) : QObject(parent), tools_(std::move(tools)) {}
SlideshowExport::~SlideshowExport() {
    if (process_ && process_->state() != QProcess::NotRunning) {
        process_->kill();
        process_->waitForFinished(3000);
    }
    if (!dir_.isEmpty())
        QDir(dir_).removeRecursively();
}
QString SlideshowExport::tool(const QString &name) const {
    const QString p = tools_ + "/" + name + ".exe";
    return QFileInfo::exists(p) ? p : QStandardPaths::findExecutable(name);
}
bool SlideshowExport::start(const QVariantList &clips, const QImage &recap, const QString &destination) {
    if (running_ || clips.isEmpty())
        return false;
    dir_ = QDir::tempPath() + "/clipforge-slideshow-" + QUuid::createUuid().toString(QUuid::Id128);
    if (!QDir().mkpath(dir_))
        return false;
    dest_ = destination;
    parts_.clear();
    done_.clear();
    current_ = -1;
    total_ = elapsed_ = 0;
    cancelled_ = false;
    // Fonts are copied next to the parts so the filter only needs plain relative names (no drive-letter escaping).
    const QString fonts = QStandardPaths::writableLocation(QStandardPaths::FontsLocation);
    const QString windowsFonts = qEnvironmentVariable("WINDIR", "C:/Windows") + "/Fonts";
    titles_ = QFile::copy(windowsFonts + "/segoeuib.ttf", dir_ + "/title.ttf") &&
              QFile::copy(windowsFonts + "/segoeui.ttf", dir_ + "/sub.ttf");
    if (!titles_)
        titles_ = QFile::copy(fonts + "/segoeuib.ttf", dir_ + "/title.ttf") && QFile::copy(fonts + "/segoeui.ttf", dir_ + "/sub.ttf");
    const QLocale fr(QLocale::French, QLocale::France);
    const int count = int(clips.size());
    for (int i = 0; i < count; ++i) {
        const auto m = clips[i].toMap();
        Part part;
        part.path = m["path"].toString();
        part.seconds = std::max(0.5, m["duration"].toDouble());
        part.title = m["game"].toString().isEmpty() ? QStringLiteral("Vidéo locale") : m["game"].toString();
        const auto when = QDateTime::fromString(m["created"].toString(), Qt::ISODateWithMs);
        part.subtitle = (when.isValid() ? fr.toString(when.date(), "dddd d MMMM yyyy") + " · " : QString()) +
                        QString("clip %1 sur %2").arg(i + 1).arg(count);
        parts_ << part;
        total_ += part.seconds;
    }
    if (!recap.isNull() && recap.save(dir_ + "/recap.png")) {
        Part still;
        still.path = dir_ + "/recap.png";
        still.seconds = kRecapSeconds;
        still.still = true;
        parts_ << still;
        total_ += still.seconds;
    }
    running_ = true;
    emit progress(0);
    step();
    return true;
}
void SlideshowExport::cancel() {
    if (!running_)
        return;
    cancelled_ = true;
    if (process_)
        process_->kill();
}
QProcess *SlideshowExport::launch(const QString &program, const QStringList &args) {
    auto *p = new QProcess(this);
    p->setWorkingDirectory(dir_);
    process_ = p;
    connect(p, &QProcess::readyReadStandardOutput, this, [this, p] {
        // ffmpeg -progress lines: out_time_us gives where the current part is.
        for (const auto &line : QString::fromUtf8(p->readAllStandardOutput()).split('\n')) {
            if (!line.startsWith("out_time_us=") || current_ < 0 || current_ >= parts_.size())
                continue;
            const double seconds = std::min(parts_[current_].seconds, line.mid(12).toDouble() / 1e6);
            if (total_ > 0)
                emit progress(std::clamp((elapsed_ + seconds) / total_ * 0.97, 0.0, 0.97));
        }
    });
    p->start(program, args);
    return p;
}
void SlideshowExport::step() {
    if (cancelled_)
        return finish(false, "Export annulé.");
    if (current_ >= 0 && current_ < parts_.size())
        elapsed_ += parts_[current_].seconds;
    if (++current_ >= parts_.size())
        return join();
    if (total_ > 0)
        emit progress(std::min(0.97, elapsed_ / total_ * 0.97));
    const Part &part = parts_[current_];
    if (part.still)
        return encode(true);
    if (!QFileInfo::exists(part.path))
        return step();
    // Imported videos can be silent: a blank track is generated for them so every part has sound.
    auto *probe = new QProcess(this);
    process_ = probe;
    connect(probe, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this, probe] {
        const bool audio = !QString::fromUtf8(probe->readAllStandardOutput()).trimmed().isEmpty();
        probe->deleteLater();
        encode(audio);
    });
    probe->start(tool("ffprobe"), {"-v", "error", "-select_streams", "a", "-show_entries", "stream=index", "-of", "csv=p=0",
                                   part.path});
}
void SlideshowExport::encode(bool audio) {
    if (cancelled_)
        return finish(false, "Export annulé.");
    const Part &part = parts_[current_];
    const QString out = QString("part-%1.mp4").arg(current_, 4, 10, QChar('0'));
    QStringList a{"-v", "error", "-nostdin", "-y"};
    QString video;
    if (part.still) {
        a << "-loop" << "1" << "-framerate" << "30" << "-t" << QString::number(kRecapSeconds) << "-i" << part.path;
        a << "-f" << "lavfi" << "-t" << QString::number(kRecapSeconds) << "-i" << "anullsrc=r=48000:cl=stereo";
        video = "[0:v]" + kFrame + ",fade=t=in:st=0:d=0.6[v]";
        a << "-filter_complex" << video << "-map" << "[v]" << "-map" << "1:a" << "-t" << QString::number(kRecapSeconds);
    } else {
        a << "-i" << part.path;
        if (!audio)
            a << "-f" << "lavfi" << "-i" << "anullsrc=r=48000:cl=stereo";
        video = "[0:v:0]" + kFrame + ",fade=t=in:st=0:d=0.25";
        if (titles_) {
            const QString n = QString::number(current_);
            QFile title(dir_ + "/title-" + n + ".txt"), subtitle(dir_ + "/sub-" + n + ".txt");
            if (title.open(QIODevice::WriteOnly) && subtitle.open(QIODevice::WriteOnly)) {
                title.write(part.title.toUtf8());
                subtitle.write(part.subtitle.toUtf8());
                title.close();
                subtitle.close();
                video += ",drawtext=fontfile=title.ttf:textfile=title-" + n +
                         ".txt:fontsize=64:fontcolor=white:shadowcolor=black@0.7:shadowx=3:shadowy=3:x=72:y=h-230:alpha=" +
                         kTitleAlpha;
                video += ",drawtext=fontfile=sub.ttf:textfile=sub-" + n +
                         ".txt:fontsize=34:fontcolor=white@0.92:shadowcolor=black@0.7:shadowx=2:shadowy=2:x=74:y=h-145:alpha=" +
                         kTitleAlpha;
            }
        }
        video += "[v]";
        a << "-filter_complex" << video << "-map" << "[v]" << "-map" << (audio ? "0:a:0" : "1:a") << "-shortest";
    }
    a << encoderArgs() << "-progress" << "pipe:1" << out;
    auto *p = launch(tool("ffmpeg"), a);
    connect(p, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, p, out](int code, QProcess::ExitStatus status) {
                const bool ok = code == 0 && status == QProcess::NormalExit && QFileInfo(dir_ + "/" + out).size() > 0;
                const QString error = QString::fromUtf8(p->readAllStandardError()).right(800);
                const bool still = parts_[current_].still;
                p->deleteLater();
                if (cancelled_)
                    return finish(false, "Export annulé.");
                if (ok)
                    done_ << out;
                else if (still) // a broken clip is skipped, but the recap must work
                    return finish(false, "Export du diaporama impossible : " + error);
                step();
            });
}
void SlideshowExport::join() {
    if (done_.isEmpty())
        return finish(false, "Aucun clip n'a pu être lu.");
    QFile list(dir_ + "/list.txt");
    if (!list.open(QIODevice::WriteOnly))
        return finish(false, "Impossible d'écrire dans le dossier temporaire.");
    for (const auto &part : done_)
        list.write(("file '" + part + "'\n").toUtf8());
    list.close();
    auto *p = launch(tool("ffmpeg"), {"-v", "error", "-nostdin", "-y", "-f", "concat", "-safe", "0", "-i", "list.txt", "-c",
                                      "copy", "-movflags", "+faststart", "slideshow.mp4"});
    connect(p, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this, p](int code, QProcess::ExitStatus status) {
        const QString error = QString::fromUtf8(p->readAllStandardError()).right(800);
        p->deleteLater();
        if (cancelled_)
            return finish(false, "Export annulé.");
        const QString built = dir_ + "/slideshow.mp4";
        if (code != 0 || status != QProcess::NormalExit || QFileInfo(built).size() == 0)
            return finish(false, "Assemblage du diaporama impossible : " + error);
        QFile::remove(dest_);
        // rename fails across drives (temp on C:, clips elsewhere), copy does not.
        if (!QFile::rename(built, dest_) && !QFile::copy(built, dest_))
            return finish(false, "Impossible d'enregistrer la vidéo dans " + QFileInfo(dest_).absolutePath());
        finish(true, {});
    });
}
void SlideshowExport::finish(bool ok, const QString &error) {
    if (!dir_.isEmpty())
        QDir(dir_).removeRecursively();
    dir_.clear();
    running_ = false;
    cancelled_ = false;
    if (ok)
        emit progress(1);
    emit finished(dest_, ok, error);
}
} // namespace cf
