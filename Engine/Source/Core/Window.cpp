#include "Window.h"

#include "Common.h"

#include "WindowsMin.h"

Illulu::Window::~Window()
{
    CloseWindow(m_hWnd);
    UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
}

void Illulu::Window::Initialize()
{
    WNDCLASSEX wndCls = RegisterWindowClass();

    constexpr WORD winStyle = WS_OVERLAPPEDWINDOW;

    RECT clientRect = {0, 0, INIT_WIDTH, INIT_HEIGHT};
    AdjustWindowRectEx(&clientRect, winStyle, false, 0);

    i32 width = GetSystemMetrics(SM_CYSCREEN);
    i32 height = GetSystemMetrics(SM_CXSCREEN);

    m_hWnd = CreateWindowEx(
        0,
        TEXT("MyWindowClass"),
        TEXT("D3D12 Window"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );
}

LRESULT Illulu::Window::WindowProc(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (uMsg)
    {
        case WM_CLOSE:
        {
            i32 res = MessageBox(hWnd, TEXT("Sure you want to exit?"), TEXT("Close"), MB_YESNO);
            if (res == IDYES)
            {
                DestroyWindow(hWnd);
            }
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;

            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rect = {0, 0, 1920, 1080};
            HBRUSH brush = CreateSolidBrush(RGB(50, 151, 151));

            FillRect(hdc, &rect, brush);

            DeleteObject(brush);

            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

WNDCLASSEX Illulu::Window::RegisterWindowClass()
{
    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = WINDOW_CLASS_NAME;

    RegisterClassEx(&wc);
}
