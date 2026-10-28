#pragma once
#include "editor/EditProject.h"
#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QSet>
namespace cf {
struct ExportOptions {
    int height = 0, fps = 60;
    QString codec = "h264", format = "mp4"; // format: mp4, gif or webp
    double maxMB = 0;                       // > 0: aim for a file under this size (Discord)
    bool vertical = false;                  // centre-crop to 9:16 after the user crop
    int crf = 20;
};
class MediaJobs : public QObject {
    Q_OBJECT
  public:
    MediaJobs(QString tools, QString cache, QObject *p = nullptr);
    ~MediaJobs();
    void import(QString path, QString game = {}, bool priority = false);
    bool exportVideo(EditProject p, QString destination, ExportOptions options);
    // Keeps only the last `seconds` of a saved replay (stream copy, so the cut lands on a keyframe).
    void trimTail(QString path, double seconds);
    void snapshot(QString source, double seconds, QString destination);
    bool busy() const {
        return exporter_ != nullptr;
    }
    int pendingImports() const {
        return int(queue_.size());
    }
    static QStringList exportArguments(const EditProject &, QString destination, const ExportOptions &);
    static double outputDuration(const EditProject &);
  signals:
    void imported(QVariantMap data);
    void thumbnail(QString path, QString image);
    void failure(QString message);
    void exportProgress(double progress);
    void exported(QString path);
    void exportBusy(bool value);
    void trimmed(QString path);
    void snapshotReady(QString path);

  private:
    struct Import {
        QString path, game;
    };
    QString tools_, cache_;
    QQueue<Import> queue_;
    QSet<QString> pending_;
    static constexpr int kParallelJobs = 3;
    int active_ = 0;
    QProcess *exporter_ = nullptr;
    QString exportTemp_;
    void next();
    void run(Import job);
    QString tool(QString name) const;
};
} // namespace cf
