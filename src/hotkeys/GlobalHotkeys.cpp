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
bool hotkey::valid(int code) {
    if (code < 0 || (code >> 16) > 7)
        return false;
    const int key = vk(code), m = mods(code);
    const bool fKey = key >= VK_F1 && key <= VK_F24;
    if (!m)
        return fKey || key == VK_PAUSE || key == VK_SNAPSHOT || key == VK_SCROLL;
    return fKey || (key >= '0' && key <= '9') || (key >= 'A' && key <= 'Z') || (key >= VK_NUMPAD0 && key <= VK_DIVIDE) ||
           (key >= VK_SPACE && key <= VK_HELP && key != VK_SELECT && key != VK_EXECUTE) || key == VK_PAUSE ||
           key == VK_SCROLL || (key >= VK_OEM_1 && key <= VK_OEM_3) || (key >= VK_OEM_4 && key <= VK_OEM_8);
}
int hotkey::fromQt(int k, int scan) {
    if (k >= Qt::Key_F1 && k <= Qt::Key_F24)
        return VK_F1 + (k - Qt::Key_F1);
    if ((k >= Qt::Key_0 && k <= Qt::Key_9) || (k >= Qt::Key_A && k <= Qt::Key_Z))
        return k;
    switch (k) {
    case Qt::Key_Space: return VK_SPACE;
    case Qt::Key_Pause: return VK_PAUSE;
    case Qt::Key_Print: return VK_SNAPSHOT;
    case Qt::Key_ScrollLock: return VK_SCROLL;
    case Qt::Key_Insert: return VK_INSERT;
    case Qt::Key_Delete: return VK_DELETE;
    case Qt::Key_Home: return VK_HOME;
    case Qt::Key_End: return VK_END;
    case Qt::Key_PageUp: return VK_PRIOR;
    case Qt::Key_PageDown: return VK_NEXT;
    case Qt::Key_Left: return VK_LEFT;
    case Qt::Key_Right: return VK_RIGHT;
    case Qt::Key_Up: return VK_UP;
    case Qt::Key_Down: return VK_DOWN;
    case Qt::Key_Control: case Qt::Key_Shift: case Qt::Key_Alt: case Qt::Key_Meta: case Qt::Key_AltGr:
    case Qt::Key_CapsLock: case Qt::Key_NumLock: case Qt::Key_Escape: case Qt::Key_Backspace:
    case Qt::Key_Return: case Qt::Key_Enter: case Qt::Key_Tab:
        return 0;
    default:
        break;
    }
    // Everything else (shifted digits, OEM keys, keypad): ask Windows what the physical key means in this layout.
    if (scan > 0) {
        const UINT v = MapVirtualKeyW(UINT(scan & 0xFF), MAPVK_VSC_TO_VK_EX);
        if (v)
            return int(v);
    }
    return 0;
}
QString hotkey::name(int code) {
    if (!code)
        return "Désactivée";
    const int key = vk(code), m = mods(code);
    QString out;
    if (m & kCtrl)
        out += "Ctrl+";
    if (m & kAlt)
        out += "Alt+";
    if (m & kShift)
        out += "Maj+";
    if (key >= VK_F1 && key <= VK_F24)
        return out + "F" + QString::number(key - VK_F1 + 1);
    if ((key >= '0' && key <= '9') || (key >= 'A' && key <= 'Z'))
        return out + QChar(key);
    LONG scan = LONG(MapVirtualKeyW(UINT(key), MAPVK_VK_TO_VSC)) << 16;
    if ((key >= VK_PRIOR && key <= VK_DOWN) || key == VK_INSERT || key == VK_DELETE || key == VK_DIVIDE)
        scan |= 1 << 24; // extended-key flag
    wchar_t text[64]{};
    if (GetKeyNameTextW(scan, text, 64) > 0)
        return out + QString::fromWCharArray(text);
    return out + QString("Touche %1").arg(key);
}
QString GlobalHotkeys::configure(int s, int marker, int mic) {
    release();
    // Shift+save is reserved for "keep the whole buffer", so save itself never carries Shift.
    const struct {
        int id, code;
        const char *label;
    } keys[] = {{kSave, s, "%1"},
                {kSaveFull, s ? (s | (hotkey::kShift << 16)) : 0, "%1"},
                {kMarker, marker, "%1 (marqueur)"},
                {kMic, mic, "%1 (micro)"}};
    QStringList busy;
    for (const auto &k : keys) {
        if (!k.code)
            continue;
        active[k.id - kSave] = hotkey::valid(k.code) &&
                               RegisterHotKey(nullptr, k.id, hotkey::mods(k.code) | MOD_NOREPEAT, hotkey::vk(k.code));
        if (!active[k.id - kSave])
            busy << QString(k.label).arg(hotkey::name(k.code));
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
