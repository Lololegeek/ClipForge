#pragma once
#include "capture/ObsEngine.h"
#include "export/MediaJobs.h"
#include "hotkeys/GlobalHotkeys.h"
#include "Updater.h"
#include "library/ClipModel.h"
#include <QJsonArray>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QVariantMap>
class QSystemTrayIcon;
namespace cf {
class Controller : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY changed)
    Q_PROPERTY(QString message READ message NOTIFY changed)
    Q_PROPERTY(QString captureMethod READ captureMethod NOTIFY changed)
    Q_PROPERTY(QString encoder READ encoder NOTIFY changed)
    Q_PROPERTY(QString currentGame READ currentGame NOTIFY changed)
    Q_PROPERTY(bool engineReady READ engineReady NOTIFY changed)
    Q_PROPERTY(bool recording READ recording NOTIFY changed)
    Q_PROPERTY(bool quitting READ quitting NOTIFY changed)
    Q_PROPERTY(QVariantMap config READ config NOTIFY configChanged)
    Q_PROPERTY(QVariantMap metrics READ metrics NOTIFY changed)
    Q_PROPERTY(QVariantList encoders READ encoders NOTIFY catalogChanged)
    Q_PROPERTY(QVariantList targets READ targets NOTIFY catalogChanged)
    Q_PROPERTY(QVariantList microphones READ microphones NOTIFY catalogChanged)
    Q_PROPERTY(QVariantList games READ games NOTIFY catalogChanged)
    Q_PROPERTY(QVariantMap selected READ selected NOTIFY selectionChanged)
    Q_PROPERTY(QVariantList segments READ segments NOTIFY editChanged)
    Q_PROPERTY(bool exportBusy READ exportBusy NOTIFY changed)
    Q_PROPERTY(double exportProgress READ exportProgress NOTIFY changed)
    Q_PROPERTY(QString lastClip READ lastClip NOTIFY changed)
    Q_PROPERTY(QString updateState READ updateState NOTIFY changed)
    Q_PROPERTY(QString updateVersion READ updateVersion NOTIFY changed)
    Q_PROPERTY(double updateProgress READ updateProgress NOTIFY changed)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY editChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY editChanged)
  public:
    Controller(QString runtime, QString data, QString tools, bool test = false);
    ~Controller();
    QString state() const {
        return state_;
    }
    QString message() const {
        return message_;
    }
    QString captureMethod() const {
        return method_;
    }
    QString encoder() const {
        return encoder_;
    }
    QString currentGame() const {
        return game_;
    }
    bool engineReady() const {
        return ready_;
    }
    bool recording() const {
        return recording_;
    }
    bool quitting() const {
        return quitting_;
    }
    QVariantMap config() const {
        return settings_.json().toVariantMap();
    }
    QVariantMap metrics() const {
        return metrics_;
    }
    QVariantList encoders() const {
        return encoders_;
    }
    QVariantList targets() const {
        return targets_;
    }
    QVariantList microphones() const {
        return microphones_;
    }
    QVariantList games() const {
        return games_;
    }
    QVariantMap selected() const {
        return selected_;
    }
    QVariantList segments() const;
    bool exportBusy() const {
        return jobs_.busy();
    }
    double exportProgress() const {
        return progress_;
    }
    QString lastClip() const {
        return lastClip_;
    }
    QString updateState() const {
        return updater_.state();
    }
    QString updateVersion() const {
        return updater_.version();
    }
    double updateProgress() const {
        return updater_.progress();
    }
    bool canUndo() const {
        return !undo_.isEmpty();
    }
    bool canRedo() const {
        return !redo_.isEmpty();
    }
    ClipModel *library() {
        return &library_;
    }
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void saveReplay();
    Q_INVOKABLE void toggleRecording();
    Q_INVOKABLE void screenshot();
    Q_INVOKABLE bool apply(QVariantMap settings);
    Q_INVOKABLE void selectClip(int index);
    Q_INVOKABLE void importVideos();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void reveal(QString path);
    Q_INVOKABLE void copyPath(QString path);
    Q_INVOKABLE void addSegment(double in, double out, double speed);
    Q_INVOKABLE void useSegment(double in, double out, double speed);
    Q_INVOKABLE void removeSegment(int index);
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void exportClip(int height, int fps, QString codec, double volume);
    Q_INVOKABLE void saveProject();
    Q_INVOKABLE void loadProject();
    Q_INVOKABLE void saveProfile(QString executable, QString name);
    Q_INVOKABLE void removeProfile(QString executable);
    Q_INVOKABLE void refreshWindows();
    Q_INVOKABLE void configurationFile(bool importing);
    Q_INVOKABLE void chooseClipsFolder();
    Q_INVOKABLE void quit();
    Q_INVOKABLE void checkForUpdates();
    Q_INVOKABLE void installUpdate();
    void runSelfTest();
    bool allowClose() const;
    void notify(QString text, bool error = false);
  signals:
    void changed();
    void configChanged();
    void catalogChanged();
    void selectionChanged();
    void editChanged();
    void showWindow();
    void openEditor();

  private:
    QString data_, state_ = "Initializing", message_, method_ = "—", encoder_ = "—", game_, lastClip_;
    Settings settings_;
    ClipModel library_;
    MediaJobs jobs_;
    GlobalHotkeys hotkeys_;
    Updater updater_;
    ObsEngine *engine_;
    QThread worker_;
    QSystemTrayIcon *tray_ = nullptr;
    QTimer detection_, updateTimer_;
    int quitAttempts_ = 0;
    bool ready_ = false, recording_ = false, test_ = false, quitting_ = false, autoStarted_ = false;
    QVariantMap metrics_, selected_;
    QVariantList encoders_, targets_, microphones_, games_;
    EditProject project_;
    QVector<EditProject> undo_, redo_;
    double progress_ = 0;
    QJsonArray testSamples_;
    void checkpoint();
    void detect();
    void scan();
    void connectTray();
};
} // namespace cf
