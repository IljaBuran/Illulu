#pragma once

#include "Common.h"
#include "WindowsMin.h"

// todo
#include <tuple>

#include "Delegate.h"


namespace Illulu
{
    class Input;

    class Window
    {
        using ClientResizeDelegate = RawDelegate<i32, i32>;

    public:

        Window() = delete;

        Window(Input& input);
        ~Window();

        void OnInitialize() noexcept;
        void Show() const noexcept;
        void OnUpdate() noexcept;
        bool ShouldClose() const noexcept;
        // todo: change the return type
        std::pair<i32, i32> GetClientSize() const noexcept;
        HWND GetNativeWindowHandle() const noexcept;

        void SetClientResizeCallback(ClientResizeDelegate callback)
        {
            m_onClientResize = callback;
        }

    private:

        LRESULT _HandleMessages(u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept;
        static LRESULT _callback_WindowProc(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept;
        static void _RegisterWindowClass() noexcept;
        void _HandleClientResize(i32 newWidth, i32 newHeight)
        {
            ILL_ASSERT(newWidth > 0 && newHeight > 0);

            m_width = newWidth;
            m_height = newHeight;

            if (m_onClientResize.IsBound())
            {
                m_onClientResize.Execute(m_width, m_height);
            }
        }

    private:

        HWND m_hWnd        = nullptr;
        i32  m_width       = 0;
        i32  m_height      = 0;
        bool m_minimized   = false;
        bool m_focused     = false;
        bool m_shouldClose = false;

        Input& m_input;

        ClientResizeDelegate m_onClientResize;

        static constexpr const wchar* WINDOW_CLASS_NAME = L"IlluluWndClass";
    };
}
