#include "Overlay.h"
#include <QApplication>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
namespace cf {
namespace {
QColor accent(Overlay::Kind k) {
    switch (k) {
    case Overlay::Success:
        return QColor("#81d9b1");
    case Overlay::Error:
        return QColor("#ffadb5");
    default:
        return QColor("#8c8cff");
    }
}
} // namespace
Overlay::Overlay(QWidget *parent) : QWidget(parent), fade_(this, "windowOpacity") {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput |
                   Qt::WindowDoesNotAcceptFocus | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(380, 76);
    hide_.setSingleShot(true);
    connect(&hide_, &QTimer::timeout, this, &Overlay::dismiss);
    spin_.setInterval(33);
    connect(&spin_, &QTimer::timeout, this, qOverload<>(&QWidget::update));
    connect(&fade_, &QPropertyAnimation::finished, this, [this] {
        if (windowOpacity() < 0.01) {
            spin_.stop();
            QWidget::hide();
        }
    });
}
void Overlay::place() {
    auto screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;
    auto r = screen->geometry();
    move(r.center().x() - width() / 2, r.top() + 48);
}
void Overlay::show(const QString &title, const QString &subtitle, Kind kind) {
    title_ = title;
    subtitle_ = subtitle;
    kind_ = kind;
    clock_.restart();
    place();
    // A pending "in progress" toast can wait for its result; anything else disappears after three seconds.
    hide_.start(kind == Progress ? 10000 : 3000);
    if (kind == Progress)
        spin_.start();
    else
        spin_.stop();
    if (!isVisible()) {
        setWindowOpacity(0);
        QWidget::show();
    }
    fade_.stop();
    fade_.setDuration(140);
    fade_.setEndValue(1.0);
    fade_.start();
    update();
}
void Overlay::dismiss() {
    fade_.stop();
    fade_.setDuration(280);
    fade_.setEndValue(0.0);
    fade_.start();
}
void Overlay::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF box = QRectF(rect()).adjusted(1, 1, -1, -1);
    p.setPen(QPen(QColor("#36394d"), 1));
    p.setBrush(QColor(23, 24, 34, 238));
    p.drawRoundedRect(box, 14, 14);
    // Accent stripe.
    QPainterPath stripe;
    stripe.addRoundedRect(QRectF(1, 1, 5, height() - 2), 2.5, 2.5);
    p.setPen(Qt::NoPen);
    p.setBrush(accent(kind_));
    p.drawPath(stripe);
    QPointF c(44, height() / 2.0);
    if (kind_ == Progress) {
        p.setPen(QPen(QColor("#36394d"), 3));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(c, 12, 12);
        p.setPen(QPen(accent(kind_), 3, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(QRectF(c.x() - 12, c.y() - 12, 24, 24), -int(clock_.elapsed() * 6 % 360) * 16, 100 * 16);
    } else {
        p.setPen(Qt::NoPen);
        p.setBrush(accent(kind_));
        p.drawEllipse(c, 14, 14);
        p.setPen(QPen(QColor("#101119"), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        if (kind_ == Error) {
            p.drawLine(c + QPointF(0, -6), c + QPointF(0, 1));
            p.drawPoint(c + QPointF(0, 6));
        } else if (kind_ == Success) {
            p.drawPolyline(QPolygonF{c + QPointF(-6, 0), c + QPointF(-2, 5), c + QPointF(6, -5)});
        } else {
            p.drawPoint(c + QPointF(0, -5));
            p.drawLine(c + QPointF(0, -1), c + QPointF(0, 6));
        }
    }
    QFont f = QApplication::font();
    f.setPixelSize(16);
    f.setWeight(QFont::DemiBold);
    p.setFont(f);
    p.setPen(QColor("#f0f1fa"));
    QRect textArea(74, 0, width() - 90, height());
    QFontMetrics fm(f);
    if (subtitle_.isEmpty()) {
        p.drawText(textArea, Qt::AlignVCenter | Qt::AlignLeft, fm.elidedText(title_, Qt::ElideRight, textArea.width()));
    } else {
        p.drawText(textArea.adjusted(0, 0, 0, -height() / 2 + 2), Qt::AlignBottom | Qt::AlignLeft,
                   fm.elidedText(title_, Qt::ElideRight, textArea.width()));
        f.setPixelSize(13);
        f.setWeight(QFont::Normal);
        p.setFont(f);
        p.setPen(QColor("#b0b3c8"));
        QFontMetrics sm(f);
        p.drawText(textArea.adjusted(0, height() / 2 + 2, 0, 0), Qt::AlignTop | Qt::AlignLeft,
                   sm.elidedText(subtitle_, Qt::ElideMiddle, textArea.width()));
    }
}
} // namespace cf
