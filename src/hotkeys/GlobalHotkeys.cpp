#include "GlobalHotkeys.h"
#include <QCoreApplication>
#include <QStringList>
#include <windows.h>
namespace cf {
GlobalHotkeys::GlobalHotkeys(QObject *p) : QObject(p) {
    QCoreApplication::instance()->installNativeEventFilter(this);
}
GlobalHotkeys::~GlobalHotkeys() {
    release();
    QCoreApplication::instance()->removeNativeEventFilter(this);
}
void GlobalHotkeys::release() {
    for (int i = 0; i < 4; ++i)
        if (active[i]) {
            UnregisterHotKey(nullptr, kSave + i);
            active[i] = false;
        }
}
QString GlobalHotkeys::configure(int s, int marker, int mic) {
    release();
    const struct {
        int id, modifiers, key;
        const char *label;
    } keys[] = {{kSave, 0, s, "F%1"},
                {kSaveFull, MOD_SHIFT, s, "Maj+F%1"},
                {kMarker, 0, marker, "F%1 (marqueur)"},
                {kMic, 0, mic, "F%1 (micro)"}};
    QStringList busy;
    for (const auto &k : keys) {
        if (!k.key)
            continue;
        active[k.id - kSave] = RegisterHotKey(nullptr, k.id, k.modifiers | MOD_NOREPEAT, k.key);
        if (!active[k.id - kSave])
            busy << QString(k.label).arg(k.key - VK_F1 + 1);
    }
    return busy.isEmpty() ? QString()
                          : busy.join(", ") + (busy.size() > 1 ? " indisponibles" : " indisponible") +
                                " (déjà utilisée ou réservée).";
}
bool GlobalHotkeys::nativeEventFilter(const QByteArray &, void *m, qintptr *) {
    auto msg = static_cast<MSG *>(m);
    if (msg->message != WM_HOTKEY || msg->wParam < kSave || msg->wParam > kMic)
        return false;
    switch (msg->wParam) {
    case kSave:
        emit save();
        break;
    case kSaveFull:
        emit saveFull();
        break;
    case kMarker:
        emit marker();
        break;
    default:
        emit mic();
    }
    return true;
}
} // namespace cf
