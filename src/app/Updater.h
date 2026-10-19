#pragma once
#include <QObject>
#include <QString>
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
    QString data_, state_ = "idle", version_, url_, digest_, file_, page_;
    double progress_ = 0;
    bool manual_ = false, download_ = false;
    void fail(const QString &text);
    void startDownload();
    void setState(const QString &s);
};
} // namespace cf
