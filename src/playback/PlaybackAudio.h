#pragma once
#include <QAudioSink>
#include <QMediaPlayer>
#include <QPointer>
#include <QProcess>
#include <QTimer>
namespace cf {
// Explicit stereo PCM avoids invalid virtual-surround channel maps.
class PlaybackAudio : public QObject {
    Q_OBJECT
    Q_PROPERTY(QObject *mediaPlayer READ mediaPlayer WRITE setMediaPlayer NOTIFY playerChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
  public:
    explicit PlaybackAudio(QObject *parent = nullptr);
    ~PlaybackAudio();
    QObject *mediaPlayer() const {
        return player_;
    }
    void setMediaPlayer(QObject *player);
    double volume() const {
        return volume_;
    }
    void setVolume(double volume);
    QString error() const {
        return error_;
    }
    qint64 processedUSecs() const {
        return processedPeak_;
    }
    Q_INVOKABLE void seek(qint64 milliseconds);
  signals:
    void playerChanged();
    void volumeChanged();
    void errorChanged();

  private:
    QPointer<QMediaPlayer> player_;
    QAudioSink *sink_ = nullptr;
    QProcess *decoder_ = nullptr;
    QIODevice *device_ = nullptr;
    QTimer drain_;
    QTimer rateRestart_, seekRestart_;
    qint64 seekTarget_ = 0;
    QByteArray pending_;
    double volume_ = 1;
    int channels_ = 2;
    qint64 processedPeak_ = 0;
    QString error_;
    void start(qint64 position = -1);
    void stopDecoder();
    void flush();
    void reset();
    void fail(QString message);
};
} // namespace cf
