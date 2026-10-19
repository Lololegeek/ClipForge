#pragma once
#include <QElapsedTimer>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWidget>
namespace cf {
// Small click-through toast shown above borderless/windowed games for a few seconds.
class Overlay : public QWidget {
    Q_OBJECT
  public:
    enum Kind { Info, Progress, Success, Error };
    explicit Overlay(QWidget *parent = nullptr);
    void show(const QString &title, const QString &subtitle = {}, Kind kind = Info);

  protected:
    void paintEvent(QPaintEvent *) override;

  private:
    QString title_, subtitle_;
    Kind kind_ = Info;
    QTimer hide_, spin_;
    QElapsedTimer clock_;
    QPropertyAnimation fade_;
    void place();
    void dismiss();
};
} // namespace cf
