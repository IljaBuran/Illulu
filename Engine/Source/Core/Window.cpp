#include "Core/Window.h"

#include "Core/Input.h"

#include "Common.h"
#include "WindowsMin.h"

#include "backends/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Illulu
{

    Window::Window(Input& input)
        : m_input(input)
    {
    }

    Window::~Window()
    {
        CloseWindow(m_hWnd);
        UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
    }

    void Window::OnInitialize() noexcept
    {
        _RegisterWindowClass();
        constexpr DWORD winStyle = WS_OVERLAPPEDWINDOW;

        // size of client window = 3/4 * the monitor's size
        i32 monitorWidth = GetSystemMetrics(SM_CXSCREEN);
        i32 monitorHeight = GetSystemMetrics(SM_CYSCREEN);

        m_width = (i32)(0.75f * monitorWidth);
        m_height = (i32)(0.75f * monitorHeight);

        RECT clientRect = {0, 0, m_width, m_height};
        AdjustWindowRectEx(&clientRect, winStyle, false, 0);

        // create window
        CreateWindowEx(
            0,
            WINDOW_CLASS_NAME,
            L"D3D12 Window",
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

        ILL_ASSERT(m_hWnd);
    }

    void Window::Show() const noexcept
    {
        ShowWindow(m_hWnd, SW_SHOW);
    }

    void Window::OnUpdate() noexcept
    {
        MSG msg{};

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    bool Window::ShouldClose() const noexcept
    {
        return m_shouldClose;
    }

    std::pair<i32, i32> Window::GetClientSize() const noexcept
    {
        return {m_width, m_height};
    }

    HWND Window::GetNativeWindowHandle() const noexcept
    {
        return m_hWnd;
    }

    LRESULT Window::_HandleMessages(u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        switch (uMsg)
        {
            case WM_KEYDOWN:
            {
                ILL_ASSERT(wParam < 0xFF);

                m_input.NotifyKeyDown(static_cast<keyCode>(wParam));
                return 0;
            }

            case WM_KEYUP:
            {
                ILL_ASSERT(wParam < 0xFF);

                m_input.NotifyKeyUp(static_cast<keyCode>(wParam));
                return 0;
            }

            [[unlikely]] case WM_CLOSE:
            {
                i32 res = MessageBox(m_hWnd, L"Sure you want to exit?", L"Close", MB_YESNO);
                if (res == IDYES)
                {
                    DestroyWindow(m_hWnd);
                    m_shouldClose = true;
                }
                return 0;
            }

            [[unlikely]] case WM_SETFOCUS:
            {
                m_focused = true;
                return 0;
            }
            [[unlikely]] case WM_KILLFOCUS:
            {
                m_focused = false;
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
                        _HandleClientResize(newWidth, newHeight);
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


    LRESULT Window::_callback_WindowProc(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        // note(ilja): normally unsafe, but we are going to have only 1 window, therefor it should NOT break
        static Window* thisWindow = nullptr;

        // note(ilja): this needs to be here for imgui
        if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return true;

        if (uMsg == WM_NCCREATE) [[unlikely]]
        {
            ILL_ASSERT(!thisWindow);

            // extract Window class instance pointer from WndClass
            CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
            thisWindow = static_cast<Window*>(cs->lpCreateParams);

            ILL_ASSERT(thisWindow);

            // error checking, it's hard to figure out if SetWindowLongPtr is successful as it returns it's previous value
            // this is some workaround

            SetLastError(0);
            ILL_VERIFY(SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(thisWindow)) || !GetLastError());

            // assign
            thisWindow->m_hWnd = hWnd;

            // let DefWindowProc handle the rest of the message
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }

        // if pointer is already assign, then call our non-static member function
        return thisWindow ? thisWindow->_HandleMessages(uMsg, wParam, lParam)
            : DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    void Window::_RegisterWindowClass() noexcept
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

        ILL_VERIFY(RegisterClassEx(&wc));
    }
}

