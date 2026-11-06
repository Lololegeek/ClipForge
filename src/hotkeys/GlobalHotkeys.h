#pragma once
#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QString>
namespace cf {
// A shortcut is one int: (modifiers << 16) | virtual-key, modifiers using the Win32 MOD_ALT/CONTROL/SHIFT bits.
// Plain F1..F24 values (0x70..0x87) are valid codes, so older settings files keep working.
namespace hotkey {
constexpr int kAlt = 1, kCtrl = 2, kShift = 4;
constexpr int make(int vk, int mods) {
    return (mods << 16) | vk;
}
constexpr int vk(int code) {
    return code & 0xFFFF;
}
constexpr int mods(int code) {
    return (code >> 16) & 7;
}
// True when `code` can be registered: a non-modifier key, and without modifiers only F-keys and a few
// dedicated keys (so typing is never hijacked).
bool valid(int code);
// Win32 virtual-key for a QML key event (Qt::Key + native scan code); 0 for modifiers and unknown keys.
int fromQt(int qtKey, int scanCode);
QString name(int code); // "Ctrl+Alt+K", "F8", "Maj+F8"...
} // namespace hotkey
class GlobalHotkeys : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
  public:
    explicit GlobalHotkeys(QObject *p = nullptr);
    ~GlobalHotkeys();
    // save: short clip; save + Shift: whole buffer. marker and mic are optional (0 = off). All are hotkey codes.
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
