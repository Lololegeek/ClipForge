#include "platform/WindowCatalog.h"
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
void ObsEngine::stop() {
    if (!replay_) {
        emit state("Idle");
        return;
    }
    if (saving_) {
        emit failure("Attendez la fin des sauvegardes avant d'arrêter.");
        return;
    }
    emit state("Stopping");
    stopping_ = true;
    queuedSaves_ = 0;
    if (record_ && obs_output_active(record_))
        obs_output_stop(record_);
    if (replay_ && obs_output_active(replay_))
        obs_output_stop(replay_);
    QTimer::singleShot(250, this, [this] {
        if ((replay_ && obs_output_active(replay_)) || (record_ && obs_output_active(record_))) {
            QTimer::singleShot(250, this, &ObsEngine::stop);
            return;
        }
        releasePipeline();
        emit state("Idle");
    });
}
void ObsEngine::releasePipeline() {
    ++replayGeneration_;
    if (watchdog_)
        watchdog_->stop();
    saving_ = false;
    queuedSaves_ = 0;
    if (record_) {
        signal_handler_disconnect(obs_output_get_signal_handler(record_), "stop", onRecordStop, this);
        if (obs_output_active(record_))
            obs_output_force_stop(record_);
        obs_output_release(record_);
        record_ = nullptr;
    }
    if (replay_) {
        signal_handler_disconnect(obs_output_get_signal_handler(replay_), "saved", onSaved, this);
        signal_handler_disconnect(obs_output_get_signal_handler(replay_), "stop", onReplayStop, this);
        if (obs_output_active(replay_))
            obs_output_force_stop(replay_);
        obs_output_release(replay_);
        replay_ = nullptr;
    }
    if (video_) {
        obs_encoder_release(video_);
        video_ = nullptr;
    }
    if (aac_) {
        obs_encoder_release(aac_);
        aac_ = nullptr;
    }
    for (int i = 0; i < 3; ++i)
        obs_set_output_source(i, nullptr);
    if (scene_) {
        obs_scene_release(scene_);
        scene_ = nullptr;
    }
    if (source_) {
        obs_source_release(source_);
        source_ = nullptr;
    }
    if (audio_) {
        obs_source_release(audio_);
        audio_ = nullptr;
    }
    if (mic_) {
        obs_source_release(mic_);
        mic_ = nullptr;
    }
    recording_ = false;
    emit recording(false);
}
void ObsEngine::stats() {
    if (!initialized_)
        return;
    PROCESS_MEMORY_COUNTERS_EX pm{};
    pm.cb = sizeof(pm);
    GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&pm), sizeof(pm));
    QVariantMap m{
        {"fps", obs_get_active_fps()},
        {"lagged", qulonglong(obs_get_lagged_frames())},
        {"total", qulonglong(obs_get_total_frames())},
        {"frameMs", obs_get_average_frame_time_ns() / 1000000.0},
        {"ramMB", double(pm.WorkingSetSize) / 1048576.0},
        {"bufferSeconds", replay_ ? std::min(qint64(settings_.replaySeconds), elapsed_.elapsed() / 1000) : 0},
        {"sourceWidth", source_ ? obs_source_get_width(source_) : 0},
        {"sourceHeight", source_ ? obs_source_get_height(source_) : 0}};
    FILETIME creation, exit, kernel, user;
    if (GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user)) {
        ULARGE_INTEGER k{}, u{};
        k.LowPart = kernel.dwLowDateTime;
        k.HighPart = kernel.dwHighDateTime;
        u.LowPart = user.dwLowDateTime;
        u.HighPart = user.dwHighDateTime;
        static quint64 last = 0;
        static ULONGLONG clock = 0;
        auto now = GetTickCount64();
        auto ticks = k.QuadPart + u.QuadPart;
        SYSTEM_INFO sys;
        GetSystemInfo(&sys);
        if (clock)
            m["cpu"] = double(ticks - last) / double(now - clock) / 100.0 / sys.dwNumberOfProcessors;
        last = ticks;
        clock = now;
    }
    emit metrics(m);
    // A missing game/window never silently broadens capture to the desktop.
    if (source_ && obs_source_get_width(source_))
        sourceWarned_ = false;
    if (replay_ && elapsed_.elapsed() > 8000 && !gameFallback_ && source_ && !obs_source_get_width(source_) &&
        settings_.mode == "game" && !settings_.target.isEmpty()) {
        // Only once the game window really exists; before that, the hook may simply be waiting for the launch.
        bool running = false;
        const QString exe = settings_.target.section(':', 2);
        for (const auto &w : windows())
            if (w.toMap().value("selector").toString().section(':', 2) == exe) {
                running = true;
                break;
            }
        gameFallback_ = running;
        if (running && fallbackGameToWindow()) {
            emit notice("Le hook de jeu n'a pas répondu : capture de la fenêtre du jeu à la place.");
            return;
        }
    }
    if (replay_ && elapsed_.elapsed() > 10000 && source_ && !obs_source_get_width(source_) &&
        !sourceWarned_) {
        sourceWarned_ = true;
        emit failure("La source n'envoie pas d'image. Vérifiez le jeu ou choisissez une fenêtre/écran.");
    }
}
void ObsEngine::shutdown() {
    if (!initialized_)
        return;
    if (timer_)
        timer_->stop();
    stopping_ = true;
    releasePipeline();
    obs_wait_for_destroy_queue();
    obs_shutdown();
    initialized_ = false;
}
} // namespace cf