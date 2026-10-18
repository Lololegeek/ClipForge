#include "ObsEngine.h"
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
ObsEngine::ObsEngine(QString r, QString c) : runtime_(std::move(r)), config_(std::move(c)) {}
ObsEngine::~ObsEngine() {
    shutdown();
}
void ObsEngine::error(const QString &m) {
    emit failure(m);
    emit state("Error");
}
void ObsEngine::initialize() {
    if (initialized_)
        return;
    QDir().mkpath(config_);

    // No module config directory: win-capture then uses its shipped compatibility data
    // without starting its optional online compatibility-file updater.
    if (!obs_startup("en-US", nullptr, nullptr)) {
        error("Impossible d'initialiser libobs. Consultez le journal.");
        return;
    }
    initialized_ = true;
    auto data = QDir::toNativeSeparators(runtime_ + "/data/libobs").toUtf8();
    obs_add_data_path(data.constData());
    // win-capture selects DXGI/WGC support during module load, so D3D must exist first.
    obs_video_info initial{};
    auto graphics = (runtime_ + "/bin/64bit/libobs-d3d11.dll").toUtf8();
    initial.graphics_module = graphics.constData();
    initial.fps_num = 30;
    initial.fps_den = 1;
    initial.base_width = 1280;
    initial.base_height = 720;
    initial.output_width = 1280;
    initial.output_height = 720;
    initial.output_format = VIDEO_FORMAT_NV12;
    initial.gpu_conversion = true;
    initial.colorspace = VIDEO_CS_709;
    initial.range = VIDEO_RANGE_PARTIAL;
    initial.scale_type = OBS_SCALE_BICUBIC;
    if (obs_reset_video(&initial) != OBS_VIDEO_SUCCESS) {
        error("Initialisation Direct3D impossible.");
        return;
    }
    for (const auto &name : QStringList{"obs-ffmpeg", "obs-x264", "win-capture", "win-wasapi", "obs-qsv11",
                                        "obs-nvenc", "image-source"}) {
        const auto dll = (runtime_ + "/obs-plugins/64bit/" + name + ".dll").toUtf8();
        const auto dir = (runtime_ + "/data/obs-plugins/" + name).toUtf8();
        obs_module_t *m = nullptr;
        if (obs_open_module(&m, dll.constData(), dir.constData()) == MODULE_SUCCESS)
            obs_init_module(m);
    }
    obs_post_load_modules();
    qInfo("OBS modules loaded");
    QVariantList enc, sources, mics;
    const char *id = nullptr;
    for (size_t i = 0; obs_enum_encoder_types(i, &id); ++i)
        if (obs_get_encoder_type(id) == OBS_ENCODER_VIDEO &&
            !(obs_get_encoder_caps(id) & OBS_ENCODER_CAP_INTERNAL))
            enc << QVariantMap{
                {"id", id}, {"name", obs_encoder_get_display_name(id)}, {"codec", obs_get_encoder_codec(id)}};
    for (const auto &type : QStringList{"monitor_capture", "window_capture"}) {
        qInfo().noquote() << "Enumerating capture properties" << type;
        const auto b = type.toUtf8();
        const auto latest = obs_get_latest_input_type_id(b.constData());
        auto p = obs_get_source_properties(latest ? latest : b.constData());
        if (!p)
            continue;
        auto list = obs_properties_get(p, type == "monitor_capture" ? "monitor_id" : "window");
        if (list)
            for (size_t i = 0; i < obs_property_list_item_count(list); ++i)
                sources << QVariantMap{{"mode", type == "monitor_capture" ? "display" : "window"},
                                       {"name", obs_property_list_item_name(list, i)},
                                       {"value", obs_property_list_item_string(list, i)}};
        obs_properties_destroy(p);
    }
    qInfo("Enumerating microphones");
    auto p = obs_get_source_properties("wasapi_input_capture");
    if (p) {
        auto d = obs_properties_get(p, "device_id");
        if (d)
            for (size_t i = 0; i < obs_property_list_item_count(d); ++i)
                mics << QVariantMap{{"name", obs_property_list_item_name(d, i)},
                                    {"value", obs_property_list_item_string(d, i)}};
        obs_properties_destroy(p);
    }
    timer_ = new QTimer(this);
    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout, this, &ObsEngine::stats);
    timer_->start();
    watchdog_ = new QTimer(this);
    watchdog_->setSingleShot(true);
    watchdog_->setInterval(30000);
    connect(watchdog_, &QTimer::timeout, this, [this] {
        if (saving_) {
            saving_ = false;
            queuedSaves_ = 0;
            emit failure("La sauvegarde n'a pas abouti en 30 s. Vérifiez le disque et le journal.");
            if (replay_ && obs_output_active(replay_))
                emit state(recording_ ? "Recording" : "Buffering");
        }
    });
    emit state("Idle");
    emit ready(enc, sources, mics);
}
} // namespace cf