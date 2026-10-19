#include "GlobalHotkeys.h"
#include <QCoreApplication>
#include <windows.h>
namespace cf {
GlobalHotkeys::GlobalHotkeys(QObject *p) : QObject(p) {
    QCoreApplication::instance()->installNativeEventFilter(this);
}
GlobalHotkeys::~GlobalHotkeys() {
    if (active)
        UnregisterHotKey(nullptr, 0xCF00);
    QCoreApplication::instance()->removeNativeEventFilter(this);
}
QString GlobalHotkeys::configure(int s) {
    if (active)
        UnregisterHotKey(nullptr, 0xCF00);
    active = RegisterHotKey(nullptr, 0xCF00, MOD_NOREPEAT, s);
    return active ? QString() : QString("F%1 indisponible (déjà utilisée ou réservée).").arg(s - VK_F1 + 1);
}
bool GlobalHotkeys::nativeEventFilter(const QByteArray &, void *m, qintptr *) {
    auto msg = static_cast<MSG *>(m);
    if (msg->message != WM_HOTKEY || msg->wParam != 0xCF00)
        return false;
    emit save();
    return true;
}
} // namespace cf
