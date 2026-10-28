#include "capture/ObsEngine.h"
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QStorageInfo>
#include <QThread>
#include <QUuid>
#include <algorithm>
#include <objbase.h>
#include <psapi.h>
#include <windows.h>
namespace cf {
bool ObsEngine::fallbackToSoftware() {
    if (stopping_ || !video_ || QString::fromUtf8(obs_encoder_get_id(video_)) == "obs_x264")
        return false;
    const bool wasRecording = recording_;
    qWarning().noquote() << "Switching failed hardware encoder to software H.264:" << encoderName_;
    settings_.encoder = "obs_x264";
    settings_.codec = "h264";
    saving_ = false;
    queuedSaves_ = 0;
    watchdog_->stop();
    emit state("Initializing");
    // Keep the existing graphics/audio devices and sources. Resetting D3D while
    // OBS is asynchronously destroying the failed encoder can deadlock.
    if (!createOutputs("obs_x264")) {
        releasePipeline();
        error("Impossible de reprendre la capture. Vérifiez l'espace disque, puis relancez le replay.");
        return true; // Failure handled; do not restore a stale Buffering state.
    }
    recording_ = false;
    emit recording(false);
    elapsed_.restart();
    emit method(captureMethod_, encoderName_);
    emit state("Buffering");
    emit notice(wasRecording
                    ? "La carte graphique a échoué : le processeur prend le relais. Enregistrement interrompu, replay vidé."
                    : "La carte graphique a échoué : le processeur prend le relais. Replay vidé.");
    return true;
}
bool ObsEngine::createOutputs(const QString &id) {
    ++replayGeneration_;
    if (replay_) {
        signal_handler_disconnect(obs_output_get_signal_handler(replay_), "saved", onSaved, this);
        signal_handler_disconnect(obs_output_get_signal_handler(replay_), "stop", onReplayStop, this);
        if (obs_output_active(replay_))
            obs_output_force_stop(replay_);
        obs_output_release(replay_);
        replay_ = nullptr;
    }
    if (record_) {
        signal_handler_disconnect(obs_output_get_signal_handler(record_), "stop", onRecordStop, this);
        if (obs_output_active(record_))
            obs_output_force_stop(record_);
        obs_output_release(record_);
        record_ = nullptr;
    }
    if (video_) {
        obs_encoder_release(video_);
        video_ = nullptr;
    }
    if (aac_) {
        obs_encoder_release(aac_);
        aac_ = nullptr;
    }
    releaseTracks();
    const auto b = id.toUtf8();
    if (!obs_get_encoder_codec(b.constData()))
        return false;
    const QString codec = obs_get_encoder_codec(b.constData());
    if ((settings_.codec == "hevc" ? "hevc" : settings_.codec) != codec)
        return false;
    auto e = obs_data_create();
    obs_data_set_int(e, "bitrate", settings_.bitrate);
    obs_data_set_int(e, "keyint_sec", 2);
    obs_data_set_string(e, "rate_control", "CBR");
    // Presets and profiles belong to the encoder: AMF's "balanced" and H.264's
    // "high" are not valid choices for every NVENC/QSV codec. Keep OBS defaults.
    if (id == "obs_x264")
        obs_data_set_string(e, "preset", "veryfast");
    else if (id == "h264_texture_amf" || id == "h265_texture_amf")
        obs_data_set_string(e, "preset", "balanced");
    video_ = obs_video_encoder_create(b.constData(), "ClipForge video encoder", e, nullptr);
    obs_data_release(e);
    if (!video_)
        return false;
    obs_encoder_set_video(video_, obs_get_video());
    e = obs_data_create();
    obs_data_set_int(e, "bitrate", 192);
    aac_ = obs_audio_encoder_create("ffmpeg_aac", "ClipForge AAC", e, 0, nullptr);
    obs_data_release(e);
    if (!aac_)
        return false;
    obs_encoder_set_audio(aac_, obs_get_audio());
    auto o = obs_data_create();
    obs_data_set_string(o, "directory", settings_.clips.toUtf8().constData());
    obs_data_set_string(o, "format", "Replay %CCYY-%MM-%DD %hh-%mm-%ss");
    obs_data_set_string(o, "extension", "mp4");
    obs_data_set_bool(o, "allow_spaces", true);
    obs_data_set_int(o, "max_time_sec", settings_.replaySeconds);
    obs_data_set_int(o, "max_size_mb", settings_.memoryMB);
    replay_ = obs_output_create("replay_buffer", "ClipForge replay", o, nullptr);
    obs_data_release(o);
    if (!replay_)
        return false;
    obs_output_set_video_encoder(replay_, video_);
    obs_output_set_audio_encoder(replay_, aac_, 0);
    if (settings_.separateTracks && audio_ && mic_) {
        // Mixer 0 keeps everything (players and Discord read the first track); mixers 1 and 2 isolate each source.
        obs_source_set_audio_mixers(audio_, 1 | 2);
        obs_source_set_audio_mixers(mic_, 1 | 4);
        const char *names[] = {"ClipForge AAC game", "ClipForge AAC microphone"};
        for (int i = 0; i < 2; ++i) {
            e = obs_data_create();
            obs_data_set_int(e, "bitrate", 160);
            tracks_[i] = obs_audio_encoder_create("ffmpeg_aac", names[i], e, size_t(i + 1), nullptr);
            obs_data_release(e);
            if (!tracks_[i])
                break;
            obs_encoder_set_audio(tracks_[i], obs_get_audio());
            obs_output_set_audio_encoder(replay_, tracks_[i], size_t(i + 1));
        }
    }
    signal_handler_connect(obs_output_get_signal_handler(replay_), "saved", onSaved, this);
    signal_handler_connect(obs_output_get_signal_handler(replay_), "stop", onReplayStop, this);
    qInfo().noquote() << "Starting replay encoder" << id << "codec" << codec;
    if (!obs_output_start(replay_)) {
        qWarning().noquote() << "Replay startup failed:" << id
                             << obs_encoder_get_last_error(video_) << obs_output_get_last_error(replay_);
        // A failed start can queue a stop event. It must not stop the next candidate.
        ++replayGeneration_;
        return false;
    }
    encoderName_ = QString::fromUtf8(obs_encoder_get_display_name(b.constData()));
    return true;
}
void ObsEngine::releaseTracks() {
    for (auto &t : tracks_)
        if (t) {
            obs_encoder_release(t);
            t = nullptr;
        }
}
void ObsEngine::setMicMuted(bool muted) {
    if (mic_)
        obs_source_set_muted(mic_, muted);
}
void ObsEngine::saveReplay() {
    if (!replay_ || !obs_output_active(replay_) || stopping_) {
        emit failure("Activez d'abord l'Instant Replay.");
        return;
    }
    qInfo().noquote() << "Replay save requested:" << encoderName_
                     << "frames" << obs_output_get_total_frames(replay_)
                     << "directory" << settings_.clips;
    if (queuedSaves_ >= 8) {
        emit failure("Huit sauvegardes sont déjà en attente.");
        return;
    }
    ++queuedSaves_;
    if (!saving_)
        requestSave();
}
void ObsEngine::requestSave() {
    if (!replay_ || queuedSaves_ == 0 || stopping_)
        return;
    --queuedSaves_;
    auto naming = obs_data_create();
    auto format =
        (QString("Replay %CCYY-%MM-%DD %hh-%mm-%ss ") + QUuid::createUuid().toString(QUuid::Id128).left(12))
            .toUtf8();
    obs_data_set_string(naming, "format", format.constData());
    obs_output_update(replay_, naming);
    obs_data_release(naming);
    saving_ = true;
    saveElapsed_.start();
    watchdog_->start();
    emit state("Saving");
    calldata_t c{};
    const bool requested = proc_handler_call(obs_output_get_proc_handler(replay_), "save", &c);
    calldata_free(&c);
    if (!requested) {
        saving_ = false;
        queuedSaves_ = 0;
        watchdog_->stop();
        if (fallbackToSoftware())
            return;
        emit failure("La demande de sauvegarde du replay a échoué. Consultez le journal.");
        emit state(recording_ ? "Recording" : "Buffering");
    }
}
void ObsEngine::onSaved(void *p, calldata_t *) {
    auto generation = static_cast<ObsEngine *>(p)->replayGeneration_.load();
    QMetaObject::invokeMethod(
        static_cast<ObsEngine *>(p),
        [p, generation] {
            auto engine = static_cast<ObsEngine *>(p);
            if (engine->replayGeneration_.load() == generation)
                engine->handleSaved();
        },
        Qt::QueuedConnection);
}
void ObsEngine::handleSaved() {
    if (!replay_)
        return;
    calldata_t c{};
    proc_handler_call(obs_output_get_proc_handler(replay_), "get_last_replay", &c);
    QString path = QString::fromUtf8(calldata_string(&c, "path"));
    calldata_free(&c);
    saving_ = false;
    watchdog_->stop();
    if (!path.isEmpty() && QFileInfo(path).size() > 0) {
        emit metrics({{"saveMs", saveElapsed_.elapsed()}});
        emit saved(path);
    } else {
        if (fallbackToSoftware())
            return;
        emit failure("Le replay n'a pas produit de fichier lisible.");
    }
    emit state(recording_ ? "Recording" : "Buffering");
    if (queuedSaves_)
        QTimer::singleShot(100, this, &ObsEngine::requestSave);
}
void ObsEngine::toggleRecording() {
    if (!replay_ || stopping_) {
        emit failure("Activez d'abord la capture.");
        return;
    }
    if (recording_) {
        obs_output_stop(record_);
        return;
    }
    if (record_) {
        obs_output_release(record_);
        record_ = nullptr;
    }
    recordPath_ = settings_.clips + "/Enregistrement " +
                  QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss-zzz") + ".mkv";
    auto o = obs_data_create();
    obs_data_set_string(o, "path", recordPath_.toUtf8().constData());
    record_ = obs_output_create("ffmpeg_muxer", "ClipForge recording", o, nullptr);
    obs_data_release(o);
    if (!record_) {
        emit failure("Impossible de créer l'enregistrement.");
        return;
    }
    obs_output_set_video_encoder(record_, video_);
    obs_output_set_audio_encoder(record_, aac_, 0);
    for (int i = 0; i < 2; ++i)
        if (tracks_[i])
            obs_output_set_audio_encoder(record_, tracks_[i], size_t(i + 1));
    signal_handler_connect(obs_output_get_signal_handler(record_), "stop", onRecordStop, this);
    if (!obs_output_start(record_)) {
        emit failure("L'enregistrement n'a pas démarré.");
        return;
    }
    recording_ = true;
    emit recording(true);
    emit state("Recording");
}
void ObsEngine::onReplayStop(void *p, calldata_t *c) {
    int code = int(calldata_int(c, "code"));
    auto generation = static_cast<ObsEngine *>(p)->replayGeneration_.load();
    QMetaObject::invokeMethod(static_cast<ObsEngine *>(p), [p, code, generation] {
        auto engine = static_cast<ObsEngine *>(p);
        if (engine->replayGeneration_.load() == generation)
            engine->handleStopped(true, code);
    }, Qt::QueuedConnection);
}
void ObsEngine::onRecordStop(void *p, calldata_t *c) {
    int code = int(calldata_int(c, "code"));
    auto generation = static_cast<ObsEngine *>(p)->replayGeneration_.load();
    QMetaObject::invokeMethod(static_cast<ObsEngine *>(p),
                              [p, code, generation] {
                                  auto engine = static_cast<ObsEngine *>(p);
                                  if (engine->replayGeneration_.load() == generation)
                                      engine->handleStopped(false, code);
                              }, Qt::QueuedConnection);
}
void ObsEngine::handleStopped(bool replay, int code) {
    if (replay) {
        if (!stopping_) {
            // Read diagnostics before releasing the output and its encoder.
            QString detail = replay_ ? QString::fromUtf8(obs_output_get_last_error(replay_)) : QString();
            if (detail.isEmpty() && video_)
                detail = QString::fromUtf8(obs_encoder_get_last_error(video_));
            qWarning().noquote() << "Replay stopped:" << code << encoderName_ << detail;
            if (code == OBS_OUTPUT_ENCODE_ERROR && fallbackToSoftware())
                return;
            releasePipeline();
            error(detail.isEmpty()
                      ? QString("Le tampon s'est arrêté (code %1). Vérifiez l'encodeur et le disque.").arg(code)
                      : QString("Le tampon s'est arrêté (code %1) : %2").arg(code).arg(detail));
        }
        return;
    }
    recording_ = false;
    emit recording(false);
    if (!code && QFileInfo(recordPath_).size() > 0)
        emit saved(recordPath_);
    else if (code)
        emit failure(
            QString("Enregistrement interrompu (code %1). Le fichier MKV peut être récupérable.").arg(code));
    if (!stopping_)
        emit state("Buffering");
}
} // namespace cf
