#include "Core/Window.h"

#include "Common.h"

#include "WindowsMin.h"

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

    // size of client window = 3/4 * the monitor's size
    i32 monitorWidth = GetSystemMetrics(SM_CXSCREEN);
    i32 monitorHeight = GetSystemMetrics(SM_CYSCREEN);

    RECT clientRect = { 0, 0,
        (i32)(0.75 * monitorWidth), (i32)(0.75 * monitorHeight) };
    AdjustWindowRectEx(&clientRect, winStyle, false, 0);

    // create window
    CreateWindowEx(
        0,
        WINDOW_CLASS_NAME,
        TEXT("D3D12 Window"),
        winStyle,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        clientRect.right - clientRect.left,
        clientRect.bottom - clientRect.top,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        reinterpret_cast<LPVOID>(this)
    );

    assert(m_hWnd);
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

bool Illulu::Window::ShouldClose() const noexcept
{
    return m_shouldClose;
}

std::pair<i32, i32> Illulu::Window::GetClientSize() const noexcept
{
    return { m_width, m_height };
}

LRESULT Illulu::Window::_HandleMessages(u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (uMsg)
    {
        [[unlikely]] case WM_CLOSE:
        {
            i32 res = MessageBox(m_hWnd, TEXT("Sure you want to exit?"), TEXT("Close"), MB_YESNO);
            if (res == IDYES)
            {
                DestroyWindow(m_hWnd);
                m_shouldClose = true;
            }
            return 0;
        }

        case WM_PAINT:
        {
            COLORREF col = m_focused ? RGB(50, 151, 151) : RGB(255, 140, 105);
            
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hWnd, &ps);
            {
                // all the painting between BeginPaint and EndPaint
                RECT rect = { 0, 0, m_width, m_height};
                HBRUSH brush = CreateSolidBrush(col);

                FillRect(hdc, &rect, brush);

                DeleteObject(brush);
            }
            EndPaint(m_hWnd, &ps);
            return 0;
        }

        [[unlikely]] case WM_SETFOCUS:
        {
            m_focused = true;

            // testing
            InvalidateRect(m_hWnd, nullptr, true);

            return 0;
        }
        [[unlikely]] case WM_KILLFOCUS:
        {
            m_focused = false;

            // testing
            InvalidateRect(m_hWnd, nullptr, true);

            return 0;
        }

        [[unlikely]] case WM_SIZE:
        {
            i32 newWidth = LOWORD(lParam);
            i32 newHeight = HIWORD(lParam);

            switch (wParam)
            {
                case SIZE_MINIMIZED:
                {
                    m_minimized = true;
                    return 0;
                }
                // SIZE_MAXIMIZED and SIZE_RESTORED are handled the same (at least for now)
                case SIZE_MAXIMIZED:
                    [[fallthrough]];
                case SIZE_RESTORED:
                {
                    m_width = newWidth;
                    m_height = newHeight;

                    m_minimized = false;
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

        [[likely]] default:
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

        // extract Window class instance pointer from WndClass
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        thisWindow = static_cast<Illulu::Window*>(cs->lpCreateParams);

        assert(thisWindow);

        // error checking, it's hard to figure out if SetWindowLongPtr is successful as it returns it's previous value
        // this is some workaround
        SetLastError(0);
        const LONG_PTR previous = SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(thisWindow));
        const DWORD error = GetLastError();

        if (!previous && error)
        {
            std::wstring errMsg = std::format(L"SetWindowLongPtr Failed ({})\n", static_cast<i32>(GetLastError()));
            OutputDebugStringW(errMsg.c_str());

            return false;
        }

        // assign 
        thisWindow->m_hWnd = hWnd;

        // let DefWindowProc handle the rest of the message
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    
    // if pointer is already assign, then call our non-static member function
    return thisWindow ? thisWindow->_HandleMessages(uMsg, wParam, lParam)
                      : DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Illulu::Window::_RegisterWindowClass() noexcept
{
    WNDCLASSEX wc
    {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = _callback_WindowProc,
        .cbClsExtra{},
        .cbWndExtra{},
        .hInstance = GetModuleHandle(nullptr),
        .hIcon{},
        .hCursor = LoadCursor(nullptr, IDC_ARROW),
        .hbrBackground{},
        .lpszMenuName{},
        .lpszClassName = WINDOW_CLASS_NAME,
        .hIconSm{}
    };

    assert(RegisterClassEx(&wc));
}
