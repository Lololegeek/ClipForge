#include <cwchar>
#include <windows.h>
LRESULT CALLBACK windowProc(HWND h, UINT message, WPARAM w, LPARAM l) {
    if (message == WM_TIMER) {
        InvalidateRect(h, nullptr, FALSE);
        return 0;
    }
    if (message == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        RECT rect;
        GetClientRect(h, &rect);
        HBRUSH bg = CreateSolidBrush(RGB(32, 34, 60));
        FillRect(dc, &rect, bg);
        DeleteObject(bg);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(210, 215, 255));
        auto text = L"ClipForge live window capture validation";
        TextOutW(dc, 24, 30, text, int(wcslen(text)));
        int x = int(GetTickCount64() / 20 % 400);
        RECT moving{x, 100, x + 100, 240};
        HBRUSH b = CreateSolidBrush(RGB(130, 130, 255));
        FillRect(dc, &moving, b);
        DeleteObject(b);
        EndPaint(h, &ps);
        return 0;
    }
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, message, w, l);
}
int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = windowProc;
    wc.hInstance = instance;
    wc.lpszClassName = L"ClipForgeFixture";
    RegisterClassW(&wc);
    auto h =
        CreateWindowExW(WS_EX_NOACTIVATE, wc.lpszClassName, L"ClipForge Capture Fixture", WS_OVERLAPPEDWINDOW,
                        -8000, -8000, 640, 360, nullptr, nullptr, instance, nullptr);
    ShowWindow(h, SW_SHOWNOACTIVATE);
    SetTimer(h, 1, 33, nullptr);
    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return 0;
}
