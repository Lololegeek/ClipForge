#pragma once
#include <QAbstractNativeEventFilter>
#include <QObject>
#include <array>
namespace cf {
class GlobalHotkeys : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
  public:
    explicit GlobalHotkeys(QObject *p = nullptr);
    ~GlobalHotkeys();
    QString configure(int save, int record, int shot);
    bool nativeEventFilter(const QByteArray &, void *, qintptr *) override;
  signals:
    void save();
    void record();
    void screenshot();

  private:
    std::array<bool, 3> active{};
};
} // namespace cf
