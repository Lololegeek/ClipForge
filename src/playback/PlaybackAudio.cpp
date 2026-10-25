#include "PlaybackAudio.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QMediaDevices>
#include <QStandardPaths>
#include <algorithm>
#include <cstring>
namespace cf {
PlaybackAudio::PlaybackAudio(QObject *parent) : QObject(parent) {
    seekRestart_.setSingleShot(true);
    seekRestart_.setInterval(60);
    connect(&seekRestart_, &QTimer::timeout, this, [this] { start(seekTarget_); });
    drain_.setInterval(5);
    connect(&drain_, &QTimer::timeout, this, &PlaybackAudio::flush);
    rateRestart_.setSingleShot(true);
    rateRestart_.setInterval(120);
    connect(&rateRestart_, &QTimer::timeout, this, [this] {
        if (!player_ || player_->playbackState() != QMediaPlayer::PlayingState)
            return;
        const auto position = player_->position();
        stopDecoder();
        start(position);
    });
}
PlaybackAudio::~PlaybackAudio() {
    reset();
}
void PlaybackAudio::setMediaPlayer(QObject *object) {
    if (player_)
        disconnect(player_, nullptr, this, nullptr);
    reset();
    player_ = qobject_cast<QMediaPlayer *>(object);
    if (!player_) {
        fail("Lecteur audio non compatible.");
        emit playerChanged();
        return;
    }
    player_->setAudioOutput(nullptr);
    connect(player_, &QMediaPlayer::sourceChanged, this, &PlaybackAudio::reset);
    connect(player_, &QMediaPlayer::playbackRateChanged, this, [this] { rateRestart_.start(); });
    connect(player_, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState s) {
        reset();
        if (s == QMediaPlayer::PlayingState)
            start();
    });
    connect(player_, &QMediaPlayer::hasAudioChanged, this, [this](bool audio) {
        if (audio && !decoder_)
            start();
    });
    emit playerChanged();
}
void PlaybackAudio::setVolume(double value) {
    volume_ = std::clamp(value, 0.0, 2.0);
    if (sink_)
        sink_->setVolume(std::min(1.0, volume_));
    emit volumeChanged();
}
void PlaybackAudio::fail(QString s) {
    if (error_ == s)
        return;
    error_ = s;
    emit errorChanged();
    qWarning().noquote() << s;
}
void PlaybackAudio::reset() {
    drain_.stop();
    rateRestart_.stop();
    seekRestart_.stop();
    stopDecoder();
    if (sink_) {
        sink_->stop();
        delete sink_;
        sink_ = nullptr;
    }
    device_ = nullptr;
}
void PlaybackAudio::stopDecoder() {
    pending_.clear();
    if (decoder_) {
        auto old = decoder_;
        decoder_ = nullptr;
        disconnect(old, nullptr, this, nullptr);
        connect(old, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), old, &QObject::deleteLater);
        if (old->state() == QProcess::NotRunning)
            old->deleteLater();
        else
            old->kill();
    }
}
void PlaybackAudio::seek(qint64 position) {
    // Dragging the seek bar fires this per pixel: move the video at once, but only start one audio decoder
    // once the cursor settles, and keep the audio sink alive instead of rebuilding it every time.
    stopDecoder();
    if (player_) {
        player_->setPosition(position);
        seekTarget_ = position;
        seekRestart_.start();
    }
}
void PlaybackAudio::start(qint64 position) {
    if (!player_ || decoder_ || player_->playbackState() != QMediaPlayer::PlayingState ||
        !player_->hasAudio() || !player_->source().isLocalFile())
        return;
    error_.clear();
    emit errorChanged();
    auto output = QMediaDevices::defaultAudioOutput();
    if (output.isNull()) {
        fail("Aucun périphérique de lecture audio disponible.");
        return;
    }
    if (!sink_) {
        QAudioFormat format = output.preferredFormat();
        format.setSampleRate(48000);
        format.setSampleFormat(QAudioFormat::Float);
        auto stereo = format;
        stereo.setChannelConfig(QAudioFormat::defaultChannelConfigForChannelCount(2));
        if (output.isFormatSupported(stereo))
            format = stereo;
        channels_ = format.channelCount();
        qInfo() << "Playback PCM" << output.description() << format << "supported"
                << output.isFormatSupported(format);
        sink_ = new QAudioSink(output, format, this);
        sink_->setBufferSize(9600 * channels_);
        sink_->setVolume(std::min(1.0, volume_));
        device_ = sink_->start();
        qInfo() << "Audio sink" << sink_->state() << sink_->error() << "free" << sink_->bytesFree();
        if (!device_) {
            fail("Impossible de lire le son sur ce périphérique.");
            reset();
            return;
        }
    }
    auto process = new QProcess;
    decoder_ = process;
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process] {
        if (decoder_ != process)
            return;
        QByteArray audio = process->readAllStandardOutput();
        if (volume_ > 1) {
            for (qsizetype offset = 0; offset + 4 <= audio.size(); offset += 4) {
                float sample;
                std::memcpy(&sample, audio.constData() + offset, 4);
                sample = float(std::clamp(double(sample) * volume_, -1.0, 1.0));
                std::memcpy(audio.data() + offset, &sample, 4);
            }
        }
        pending_ += audio;
        const qsizetype limit = 192000 * channels_;
        if (pending_.size() > limit) {
            pending_.remove(0, pending_.size() - limit);
            fail("Lecture audio ralentie par le périphérique ; tampon limité à 1 s.");
        }
        flush();
    });
    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error) {
        if (decoder_ == process && error == QProcess::FailedToStart)
            fail("Le décodeur audio FFmpeg est introuvable.");
    });
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, process](int code, QProcess::ExitStatus status) {
                if (decoder_ == process && (code || status != QProcess::NormalExit))
                    fail("Décodage audio impossible : " +
                         QString::fromUtf8(process->readAllStandardError()).right(400));
            });
    double rate = std::clamp(player_->playbackRate(), 0.25, 4.0);
    QString tempo;
    if (rate < 0.5)
        tempo = "atempo=0.5,atempo=" + QString::number(rate / 0.5);
    else if (rate > 2)
        tempo = "atempo=2,atempo=" + QString::number(rate / 2);
    else
        tempo = "atempo=" + QString::number(rate);
    auto path = QCoreApplication::applicationDirPath() + "/tools/ffmpeg.exe";
    if (!QFileInfo::exists(path))
        path = QStandardPaths::findExecutable("ffmpeg");
    process->start(path,
                   {"-v",
                    "error",
                    "-nostdin",
                    "-probesize",
                    "131072",
                    "-analyzeduration",
                    "0",
                    "-readrate",
                    QString::number(rate),
                    "-ss",
                    QString::number(double(position >= 0 ? position : player_->position()) / 1000.0, 'f', 3),
                    "-i",
                    player_->source().toLocalFile(),
                    "-map",
                    "0:a:0",
                    "-vn",
                    "-af",
                    tempo,
                    "-ac",
                    QString::number(channels_),
                    "-ar",
                    "48000",
                    "-f",
                    "f32le",
                    "pipe:1"});
    drain_.start();
}
void PlaybackAudio::flush() {
    if (!sink_ || !device_ || !player_ || player_->playbackState() != QMediaPlayer::PlayingState)
        return;
    auto bytes = std::min(sink_->bytesFree(), pending_.size());
    bytes -= bytes % (channels_ * 4);
    if (bytes <= 0)
        return;
    auto written = device_->write(pending_.constData(), bytes);
    processedPeak_ = std::max(processedPeak_, sink_->processedUSecs());
    if (written > 0)
        pending_.remove(0, written);
    else if (written < 0)
        fail("Échec du périphérique de lecture audio.");
}
} // namespace cf
