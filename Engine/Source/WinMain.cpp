#include "Common.h"

#include <windows.h>

#include <format>
#include <string>

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 619;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

constexpr static auto WindowClassName = TEXT("IlluluWindowClass");

static void RegisterWindowClass(HINSTANCE hInstance)
{
    static bool registered = false;

    if (registered)
    {
        return;
    }

    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = WindowClassName;

    RegisterClassEx(&wc);

    registered = true;
}

static HWND CreateViewportWindow(HWND parent, i32 width, i32 height)
{
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    RegisterWindowClass(hInstance);

    HWND hwnd = CreateWindowEx(
        0,
        WindowClassName,
        L"Engine Viewport",
        WS_CHILD | WS_VISIBLE,
        0,
        0,
        width,
        height,
        parent,
        nullptr,
        hInstance,
        nullptr
    );

    return hwnd;
}


static LRESULT WindowProc(HWND hwnd, u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (uMsg)
    {
        case WM_CLOSE:
        {
            i32 res = MessageBox(hwnd, TEXT("Sure you want to exit?"), TEXT("Close"), MB_YESNO);
            if (res == IDYES)
            {
                DestroyWindow(hwnd);
            }
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;

            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect = {0, 0, 1920, 1080};
            HBRUSH brush = CreateSolidBrush(RGB(50, 151, 151));

            FillRect(hdc, &rect, brush);

            DeleteObject(brush);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }

        case WM_LBUTTONDBLCLK:
        {
            i16 x = LOWORD(lParam);
            i16 y = HIWORD(lParam);

            std::wstring str = std::format(L"{} {}\n", x, y);

            OutputDebugString(str.c_str());

            return 0;
        }

    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

i32 WinMain(
    HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPSTR lpCmdLine,
    i32 nCmdShow)
{
    WNDCLASSEX windowClass
    {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WindowProc,
        .cbClsExtra{},
        .cbWndExtra{},
        .hInstance = hInstance,
        .hIcon{},
        .hCursor = LoadCursor(nullptr, IDC_ARROW), // note(ilja): this fixes weird cursor behaviour
        .hbrBackground{},
        .lpszMenuName{},
        .lpszClassName = TEXT("MyWindowClass"),
        .hIconSm{}
    };

    RegisterClassEx(&windowClass);

    HWND hwnd = CreateWindowEx(
        0,
        TEXT("MyWindowClass"),
        TEXT("D3D12 Window"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1920,
        1080,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (hwnd == nullptr)
    {
        return GetLastError();
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg{};

    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}