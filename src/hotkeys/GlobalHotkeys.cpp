#include "GlobalHotkeys.h"
#include <QCoreApplication>
#include <windows.h>
namespace cf {
GlobalHotkeys::GlobalHotkeys(QObject *p) : QObject(p) {
    QCoreApplication::instance()->installNativeEventFilter(this);
}
GlobalHotkeys::~GlobalHotkeys() {
    for (int i = 0; i < 3; ++i)
        if (active[i])
            UnregisterHotKey(nullptr, 0xCF00 + i);
    QCoreApplication::instance()->removeNativeEventFilter(this);
}
QString GlobalHotkeys::configure(int s, int r, int t) {
    for (int i = 0; i < 3; ++i) {
        if (active[i])
            UnregisterHotKey(nullptr, 0xCF00 + i);
        active[i] = false;
    }
    int keys[]{s, r, t};
    QStringList errors;
    for (int i = 0; i < 3; ++i) {
        active[i] = RegisterHotKey(nullptr, 0xCF00 + i, MOD_NOREPEAT, keys[i]);
        if (!active[i])
            errors << QString("F%1 indisponible (déjà utilisée ou réservée).").arg(keys[i] - VK_F1 + 1);
    }
    return errors.join("\n");
}
bool GlobalHotkeys::nativeEventFilter(const QByteArray &, void *m, qintptr *) {
    auto msg = static_cast<MSG *>(m);
    if (msg->message != WM_HOTKEY)
        return false;
    switch (msg->wParam) {
    case 0xCF00:
        emit save();
        return true;
    case 0xCF01:
        emit record();
        return true;
    case 0xCF02:
        emit screenshot();
        return true;
    }
    return false;
}
} // namespace cf
