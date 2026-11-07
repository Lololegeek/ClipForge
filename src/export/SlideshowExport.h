#pragma once
#include <QImage>
#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QVariantList>
namespace cf {
// Builds one MP4 from a list of clips: every clip is brought to 1080p 30 fps with its title shown at the start,
// the recap image is added at the end, then the parts are joined without re-encoding.
class SlideshowExport : public QObject {
    Q_OBJECT
  public:
    explicit SlideshowExport(QString tools, QObject *parent = nullptr);
    ~SlideshowExport();
    bool busy() const {
        return running_;
    }
    // clips: maps with path, game, created (ISO) and duration, in play order.
    bool start(const QVariantList &clips, const QImage &recap, const QString &destination);
    void cancel();
    static constexpr double kRecapSeconds = 8;
  signals:
    void progress(double value);
    void finished(QString destination, bool ok, QString error);

  private:
    struct Part {
        QString path, title, subtitle;
        double seconds = 0;
        bool still = false;
    };
    QString tools_, dir_, dest_;
    QList<Part> parts_;
    QStringList done_;
    int current_ = -1;
    double total_ = 0, elapsed_ = 0;
    bool running_ = false, cancelled_ = false, titles_ = false;
    QPointer<QProcess> process_;
    QString tool(const QString &name) const;
    void step();
    void encode(bool audio);
    void join();
    void finish(bool ok, const QString &error);
    QProcess *launch(const QString &program, const QStringList &args);
};
} // namespace cf
