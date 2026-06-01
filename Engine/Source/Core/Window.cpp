#include "Window.h"

#include "Common.h"

#include "WindowsMin.h"

#include <stdexcept>
#include <string>
#include <format>
#include <cassert>

Illulu::Window::~Window()
{
    CloseWindow(m_hWnd);
    UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
}

void Illulu::Window::Initialize() noexcept
{
    _RegisterWindowClass();

    constexpr DWORD winStyle = WS_OVERLAPPEDWINDOW;

    RECT clientRect = {0, 0, (LONG)1280, (LONG)720};
    AdjustWindowRectEx(&clientRect, winStyle, false, 0);

    SetLastError(0);

    m_hWnd = CreateWindowEx(
        0,
        WINDOW_CLASS_NAME,
        TEXT("D3D12 Window"),
        winStyle,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1280,
        720,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        reinterpret_cast<LPVOID>(this)
    );

    if (!m_hWnd)
    {
        OutputDebugString(std::wstring(std::format(L"{}", GetLastError())).c_str());
    }
}

void Illulu::Window::Show() const noexcept
{
    ShowWindow(m_hWnd, SW_SHOW);
}

void Illulu::Window::PullMessages() noexcept
{
    MSG msg{};

    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT Illulu::Window::HandleMessages(u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (uMsg)
    {
        [[unlikely]] case WM_CLOSE:
        {
            i32 res = MessageBox(m_hWnd, TEXT("Sure you want to exit?"), TEXT("Close"), MB_YESNO);
            if (res == IDYES)
            {
                DestroyWindow(m_hWnd);
            }
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;

            HDC hdc = BeginPaint(m_hWnd, &ps);
            RECT rect = { 0, 0, 1920, 1080 };
            HBRUSH brush = CreateSolidBrush(RGB(50, 151, 151));

            FillRect(hdc, &rect, brush);

            DeleteObject(brush);

            EndPaint(m_hWnd, &ps);
            return 0;
        }

        [[unlikely]] case WM_SIZE:
        {
            i32 newWidth = LOWORD(lParam);
            i32 newHeight = HIWORD(lParam);
            std::wstring str;

            switch (wParam)
            {
            case SIZE_MINIMIZED:
            {
                // m_minimized = true;

                str = std::format(L"SIZE_MINIMIZED ({}, {})\n", newWidth, newHeight);
                OutputDebugString(str.c_str());
                return 0;
            }
            case SIZE_MAXIMIZED:
            {
                str = std::format(L"SIZE_MAXIMIZED ({}, {})\n", newWidth, newHeight);
                OutputDebugString(str.c_str());
                return 0;
            }
            case SIZE_RESTORED:
            {
                str = std::format(L"SIZE_RESTORED ({}, {})\n", newWidth, newHeight);
                OutputDebugString(str.c_str());
                return 0;
            }
            }
            return 0;
        }

        [[unlikely]] case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }

        default:
        {
            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }
    }
}

LRESULT Illulu::Window::_callback_WindowProc(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    // note(ilja): normally unsafe, but we are going to have only 1 window, therefor it should NOT break
    static Illulu::Window* thisWindow = nullptr;

    if (uMsg == WM_NCCREATE) [[unlikely]]
    {
        assert(!thisWindow);

        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        thisWindow = static_cast<Illulu::Window*>(cs->lpCreateParams);

        assert(thisWindow);

        SetLastError(0);
        const LONG_PTR previous = SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(thisWindow));

        const DWORD error = GetLastError();

        if (!previous && error)
        {
            std::wstring errMsg = std::format(L"SetWindowLongPtr Failed ({})\n", static_cast<i32>(GetLastError()));
            OutputDebugStringW(errMsg.c_str());

            return false;
        }

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    
    return thisWindow ? thisWindow->HandleMessages(uMsg, wParam, lParam)
                      : DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Illulu::Window::_RegisterWindowClass() noexcept
{
    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = _callback_WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = WINDOW_CLASS_NAME;

    assert(RegisterClassEx(&wc));
}
