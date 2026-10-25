#pragma once
#include "editor/EditProject.h"
#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QSet>
namespace cf {
class MediaJobs : public QObject {
    Q_OBJECT
  public:
    MediaJobs(QString tools, QString cache, QObject *p = nullptr);
    ~MediaJobs();
    void import(QString path, QString game = {}, bool priority = false);
    bool exportVideo(EditProject p, QString destination, int height, int fps, QString codec);
    bool busy() const {
        return exporter_ != nullptr;
    }
    int pendingImports() const {
        return int(queue_.size());
    }
    static QStringList exportArguments(const EditProject &, QString destination, int height, int fps,
                                       QString codec);
  signals:
    void imported(QVariantMap data);
    void thumbnail(QString path, QString image);
    void failure(QString message);
    void exportProgress(double progress);
    void exported(QString path);
    void exportBusy(bool value);

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
