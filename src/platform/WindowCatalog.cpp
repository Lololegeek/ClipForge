#include "WindowCatalog.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <windows.h>
namespace cf {
static QString executable(DWORD pid) {
    HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!p)
        return {};
    wchar_t path[32768];
    DWORD n = 32768;
    QString s;
    if (QueryFullProcessImageNameW(p, 0, path, &n))
        s = QString::fromWCharArray(path, int(n));
    CloseHandle(p);
    return s;
}
QVariantList windows() {
    QVariantList list;
    EnumWindows(
        [](HWND w, LPARAM data) -> BOOL {
            if (!IsWindowVisible(w) || GetWindow(w, GW_OWNER) || GetWindowTextLengthW(w) == 0)
                return TRUE;
            DWORD pid;
            GetWindowThreadProcessId(w, &pid);
            if (pid == GetCurrentProcessId())
                return TRUE;
            wchar_t title[1024], cls[256];
            GetWindowTextW(w, title, 1024);
            GetClassNameW(w, cls, 256);
            const auto path = executable(pid);
            const auto name = QFileInfo(path).fileName();
            if (name.isEmpty())
                return TRUE;
            QString t = QString::fromWCharArray(title);
            auto encode = [](QString v) { return v.replace("#", "#22").replace(":", "#3A"); };
            QString selector = encode(t) + ":" + encode(QString::fromWCharArray(cls)) + ":" + encode(name);
            reinterpret_cast<QVariantList *>(data)->append(QVariantMap{{"title", t},
                                                                       {"executable", path},
                                                                       {"name", name},
                                                                       {"selector", selector},
                                                                       {"pid", qulonglong(pid)}});
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&list));
    return list;
}
QString foregroundExecutable() {
    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    return executable(pid);
}
} // namespace cf
