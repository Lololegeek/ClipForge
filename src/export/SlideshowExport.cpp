#include "SlideshowExport.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QPainter>
#include <QPainterPath>
#include <QStandardPaths>
#include <QUuid>
namespace cf {
namespace {
// The bundled ffmpeg is a slim build (no drawtext, no lavfi): titles are drawn here as transparent pictures
// laid over the clip, and silence comes from a small raw PCM file looped as needed.
constexpr double kTitleSeconds = 3.8;
bool drawTitle(const QString &path, const QString &title, const QString &subtitle) {
    QImage img(1920, 1080, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    // Soft dark band at the bottom so white text stays readable on bright clips.
    QLinearGradient shade(0, 700, 0, 1080);
    shade.setColorAt(0, QColor(0, 0, 0, 0));
    shade.setColorAt(1, QColor(0, 0, 0, 150));
    p.fillRect(0, 700, 1920, 380, shade);
    auto text = [&p](const QString &s, int pixelSize, int weight, int x, int y, int alpha, int offset) {
        QFont f("Segoe UI");
        f.setPixelSize(pixelSize);
        f.setWeight(QFont::Weight(weight));
        p.setFont(f);
        const QString line = QFontMetrics(f).elidedText(s, Qt::ElideRight, 1920 - 2 * x);
        p.setPen(QColor(0, 0, 0, 170));
        p.drawText(x + offset, y + offset, line);
        p.setPen(QColor(255, 255, 255, alpha));
        p.drawText(x, y, line);
    };
    text(title, 64, QFont::Bold, 72, 905, 255, 3);
    text(subtitle, 34, QFont::Normal, 74, 965, 235, 2);
    p.end();
    return img.save(path);
}
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
    // One second of 48 kHz stereo silence, looped for silent clips and the recap.
    QFile silence(dir_ + "/silence.raw");
    titles_ = silence.open(QIODevice::WriteOnly) && silence.write(QByteArray(48000 * 4, '\0')) == 48000 * 4;
    silence.close();
    if (!titles_)
        return false;
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
    const QStringList silence{"-f", "s16le", "-ar", "48000", "-ac", "2", "-stream_loop", "-1", "-i", "silence.raw"};
    const QString seconds = QString::number(part.still ? kRecapSeconds : part.seconds, 'f', 3);
    if (part.still) {
        a << "-loop" << "1" << "-framerate" << "30" << "-t" << seconds << "-i" << part.path << silence;
        a << "-filter_complex" << "[0:v]" + kFrame + ",fade=t=in:st=0:d=0.6[v]" << "-map" << "[v]" << "-map" << "1:a";
    } else {
        a << "-i" << part.path;
        const QString n = QString::number(current_);
        const bool title = drawTitle(dir_ + "/title-" + n + ".png", part.title, part.subtitle);
        if (title)
            a << "-loop" << "1" << "-framerate" << "30" << "-t" << QString::number(kTitleSeconds) << "-i" << "title-" + n + ".png";
        if (!audio)
            a << silence;
        QString video = "[0:v:0]" + kFrame + ",fade=t=in:st=0:d=0.25";
        if (title)
            video += "[base];[1:v]format=rgba,fade=t=in:st=0:d=0.4:alpha=1,fade=t=out:st=3.2:d=0.6:alpha=1[t];"
                     "[base][t]overlay=0:0:eof_action=pass:format=auto,format=yuv420p";
        video += "[v]";
        a << "-filter_complex" << video << "-map" << "[v]" << "-map" << (audio ? "0:a:0" : QString::number(title ? 2 : 1) + ":a");
    }
    a << "-t" << seconds;
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
