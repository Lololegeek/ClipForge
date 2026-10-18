#pragma once
#include "settings/Settings.h"
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <atomic>
#include <obs.h>
namespace cf {
class ObsEngine : public QObject {
    Q_OBJECT
  public:
    explicit ObsEngine(QString runtime, QString config);
    ~ObsEngine();
  public slots:
    void initialize();
    void start(cf::Settings settings);
    void stop();
    void saveReplay();
    void toggleRecording();
    void stats();
    void shutdown();
  signals:
    void ready(QVariantList encoders, QVariantList sources, QVariantList microphones);
    void state(QString state);
    void failure(QString message);
    void saved(QString path);
    void metrics(QVariantMap data);
    void method(QString method, QString encoder);
    void recording(bool active);

  private:
    QString runtime_, config_, recordPath_, encoderName_, captureMethod_;
    Settings settings_;
    bool initialized_ = false, recording_ = false, stopping_ = false, saving_ = false, sourceWarned_ = false;
    int queuedSaves_ = 0;
    std::atomic<quint64> replayGeneration_{0};
    QElapsedTimer elapsed_, saveElapsed_;
    QTimer *timer_ = nullptr;
    QTimer *watchdog_ = nullptr;
    obs_source_t *source_ = nullptr, *audio_ = nullptr, *mic_ = nullptr;
    obs_scene_t *scene_ = nullptr;
    obs_encoder_t *video_ = nullptr, *aac_ = nullptr;
    obs_output_t *replay_ = nullptr, *record_ = nullptr;
    bool createOutputs(const QString &);
    void releasePipeline();
    void requestSave();
    void handleSaved();
    void handleStopped(bool replay, int code);
    void error(const QString &);
    static void onSaved(void *, calldata_t *);
    static void onReplayStop(void *, calldata_t *);
    static void onRecordStop(void *, calldata_t *);
};
} // namespace cf
Q_DECLARE_METATYPE(cf::Settings)
