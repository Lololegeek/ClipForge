#pragma once
#include <QAbstractNativeEventFilter>
#include <QObject>
namespace cf {
class GlobalHotkeys : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
  public:
    explicit GlobalHotkeys(QObject *p = nullptr);
    ~GlobalHotkeys();
    QString configure(int save);
    bool nativeEventFilter(const QByteArray &, void *, qintptr *) override;
  signals:
    void save();

  private:
    bool active = false;
};
} // namespace cf
