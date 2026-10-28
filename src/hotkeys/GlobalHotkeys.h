#pragma once
#include <QAbstractNativeEventFilter>
#include <QObject>
namespace cf {
class GlobalHotkeys : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
  public:
    explicit GlobalHotkeys(QObject *p = nullptr);
    ~GlobalHotkeys();
    // save: short clip; Shift+save: whole buffer. marker and mic are optional (0 = off).
    QString configure(int save, int marker = 0, int mic = 0);
    bool nativeEventFilter(const QByteArray &, void *, qintptr *) override;
  signals:
    void save();
    void saveFull();
    void marker();
    void mic();

  private:
    static constexpr int kSave = 0xCF00, kSaveFull = 0xCF01, kMarker = 0xCF02, kMic = 0xCF03;
    bool active[4]{};
    void release();
};
} // namespace cf
