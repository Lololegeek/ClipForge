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
void ObsEngine::start(Settings s) {
    if (!initialized_) {
        error("Moteur indisponible.");
        return;
    }
    if (replay_) {
        emit failure("Arrêtez la capture avant de changer la configuration.");
        return;
    }
    const auto validation = s.validate();
    if (!validation.isEmpty()) {
        error(validation);
        return;
    }
    settings_ = s;
    sourceWarned_ = false;
    emit state("Initializing");
    if (!QDir().mkpath(s.clips) || QStorageInfo(s.clips).bytesAvailable() < 256LL * 1024 * 1024) {
        error("Dossier inaccessible ou moins de 256 Mo disponibles.");
        return;
    }
    obs_video_info vi{};
    auto graphics = (runtime_ + "/bin/64bit/libobs-d3d11.dll").toUtf8();
    vi.graphics_module = graphics.constData();
    vi.fps_num = s.fps;
    vi.fps_den = 1;
    vi.base_width = s.width;
    vi.base_height = s.height;
    vi.output_width = s.width;
    vi.output_height = s.height;
    vi.output_format = VIDEO_FORMAT_NV12;
    vi.adapter = 0;
    vi.gpu_conversion = true;
    vi.colorspace = VIDEO_CS_709;
    vi.range = VIDEO_RANGE_PARTIAL;
    vi.scale_type = OBS_SCALE_BICUBIC;
    if (obs_reset_video(&vi) != OBS_VIDEO_SUCCESS) {
        error("Initialisation Direct3D 11 impossible pour cette résolution/fréquence.");
        return;
    }
    obs_audio_info ai{};
    ai.samples_per_sec = 48000;
    ai.speakers = SPEAKERS_STEREO;
    if (!obs_reset_audio(&ai)) {
        error("Initialisation audio 48 kHz impossible.");
        return;
    }
    auto data = obs_data_create();
    obs_data_set_bool(data, "capture_cursor", true);
    const char *type = s.mode == "display"  ? "monitor_capture"
                       : s.mode == "window" ? "window_capture"
                                            : "game_capture";
    if (s.mode == "display") {
        obs_data_set_string(data, "monitor_id", s.target.toUtf8().constData());
        obs_data_set_int(data, "method", 1);
        captureMethod_ = "Écran · DXGI";
    } else {
        obs_data_set_string(data, "window", s.target.toUtf8().constData());
        obs_data_set_int(data, "priority", 1);
        if (s.mode == "window") {
            bool wgc = true;
            auto props = obs_get_source_properties(obs_get_latest_input_type_id("window_capture"));
            if (props) {
                auto methods = obs_properties_get(props, "method");
                if (methods)
                    for (size_t i = 0; i < obs_property_list_item_count(methods); ++i)
                        if (obs_property_list_item_int(methods, i) == 2)
                            wgc = !obs_property_list_item_disabled(methods, i);
                obs_properties_destroy(props);
            }
            obs_data_set_int(data, "method", wgc ? 2 : 1);
            captureMethod_ =
                wgc ? "Fenêtre · Windows Graphics Capture" : "Fenêtre · BitBlt (repli WGC indisponible)";
        } else {
            obs_data_set_string(data, "capture_mode", s.target.isEmpty() ? "any_fullscreen" : "window");
            obs_data_set_bool(data, "anti_cheat_hook", true);
            captureMethod_ = "Jeu · hook OBS autorisé";
        }
    }
    const char *latest = obs_get_latest_input_type_id(type);
    source_ = obs_source_create(latest ? latest : type, "ClipForge video", data, nullptr);
    obs_data_release(data);
    if (!source_) {
        releasePipeline();
        error("Cette source de capture n'est pas disponible. Essayez Écran ou Fenêtre.");
        return;
    }
    scene_ = obs_scene_create_private("ClipForge scene");
    auto item = obs_scene_add(scene_, source_);
    vec2 bounds{float(s.width), float(s.height)};
    obs_sceneitem_set_bounds_type(item, OBS_BOUNDS_SCALE_INNER);
    obs_sceneitem_set_bounds(item, &bounds);
    obs_sceneitem_set_bounds_alignment(item, OBS_ALIGN_CENTER);
    obs_set_output_source(0, obs_scene_get_source(scene_));
    if (s.audio != "disabled") {
        auto a = obs_data_create();
        obs_data_set_string(a, "device_id", "default");
        const char *audioType = "wasapi_output_capture";
        if (s.audio == "process") {
            if (s.target.isEmpty() || s.mode == "display") {
                obs_data_release(a);
                releasePipeline();
                error("L'audio par application exige une fenêtre ou un jeu sélectionné.");
                return;
            }
            audioType = "wasapi_process_output_capture";
            obs_data_set_string(a, "window", s.target.toUtf8().constData());
            obs_data_set_int(a, "priority", 1);
        }
        audio_ = obs_source_create(audioType, "ClipForge audio", a, nullptr);
        obs_data_release(a);
        if (!audio_) {
            releasePipeline();
            error("Capture audio demandée indisponible. Sélectionnez explicitement une autre source audio.");
            return;
        }
        obs_source_set_volume(audio_, float(s.systemVolume));
        obs_set_output_source(1, audio_);
    }
    if (s.microphone != "disabled") {
        auto a = obs_data_create();
        obs_data_set_string(a, "device_id", s.microphone.toUtf8().constData());
        mic_ = obs_source_create("wasapi_input_capture", "ClipForge microphone", a, nullptr);
        obs_data_release(a);
        if (!mic_) {
            releasePipeline();
            error("Microphone indisponible.");
            return;
        }
        obs_source_set_volume(mic_, float(s.micVolume));
        obs_set_output_source(2, mic_);
    }
    QStringList candidates;
    if (s.encoder != "auto")
        candidates << s.encoder;
    if (s.encoder == "obs_x264") {
        settings_.codec = "h264";
    } else if (s.codec == "h264")
        candidates << "h264_texture_amf" << "obs_nvenc_h264_tex" << "obs_qsv11_v2" << "obs_x264";
    else if (s.codec == "hevc")
        candidates << "h265_texture_amf" << "obs_nvenc_hevc_tex" << "obs_qsv11_hevc";
    else
        candidates << "av1_texture_amf" << "obs_nvenc_av1_tex" << "obs_qsv11_av1";
    candidates.removeDuplicates();
    bool ok = false;
    for (const auto &candidate : candidates) {
        if (createOutputs(candidate)) {
            ok = true;
            break;
        }
    }
    if (!ok && s.encoder != "obs_x264") {
        settings_.codec = "h264";
        ok = createOutputs("obs_x264");
    }
    if (!ok) {
        releasePipeline();
        error("Aucun encodeur compatible n'a démarré. Essayez H.264, 1080p, 30 FPS.");
        return;
    }
    stopping_ = false;
    elapsed_.start();
    emit method(captureMethod_, encoderName_);
    emit state("Buffering");
    if (video_ && QString::fromUtf8(obs_encoder_get_id(video_)) == "obs_x264" && s.encoder != "obs_x264")
        emit notice("Encodeur matériel indisponible : capture en H.264 logiciel (CPU).");
}
} // namespace cf
