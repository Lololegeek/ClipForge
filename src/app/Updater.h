#pragma once
#include <QHash>
#include <QObject>
#include <QPair>
#include <QString>
#include <vector>
class QNetworkAccessManager;
namespace cf {
// Checks GitHub releases, downloads the installer and runs it silently on request.
class Updater : public QObject {
    Q_OBJECT
  public:
    explicit Updater(QString data, QObject *parent = nullptr);
    QString state() const {
        return state_;
    }
    QString version() const {
        return version_;
    }
    double progress() const {
        return progress_;
    }
    // Release notes (GitHub release body, Markdown).
    QString notes() const {
        return notes_;
    }
    // Bytes this update downloads: the installer, or only the changed files once the delta is known.
    qint64 downloadSize() const {
        return downloadSize_;
    }
    bool delta() const {
        return delta_;
    }
    // Only installer-based installs can update themselves; portable copies open the release page.
    bool selfUpdatable() const;
    void check(bool manual, bool download);
    void downloadNow();
    bool ready() const {
        return state_ == "ready";
    }
    // Starts the downloaded installer; the caller must quit right after.
    bool launchInstaller();
    static int compare(const QString &a, const QString &b);
  signals:
    void changed();
    void message(QString text, bool error);

  private:
    QNetworkAccessManager *net_;
    // A file of the release ZIP: its compressed bytes are [offset, offset + csize) of the archive.
    struct Entry {
        QString path, sha;
        qint64 size = 0, csize = 0, offset = 0;
        int method = 0; // 0 stored, 8 deflate
    };
    // One HTTP Range request covering one or more neighbouring missing files.
    struct Job {
        qint64 start = 0, length = 0;
        std::vector<Entry> files;
    };
    QString data_, state_ = "idle", version_, url_, digest_, file_, page_, manifestUrl_, notes_;
    QHash<QString, QPair<QString, qint64>> assets_; // release asset name -> (url, size)
    qint64 installerSize_ = 0, downloadSize_ = 0;
    QString archiveUrl_;
    std::vector<Job> missing_;
    qint64 deltaTotal_ = 0, deltaDone_ = 0;
    bool delta_ = false;
    void startDelta();
    void downloadNextBlob(size_t index);
    bool launchDelta();
    double progress_ = 0;
    bool manual_ = false, download_ = false;
    void fail(const QString &text);
    void startDownload();
    void startFullDownload();
    void setState(const QString &s);
};
} // namespace cf
