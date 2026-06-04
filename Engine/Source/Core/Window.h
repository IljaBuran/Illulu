#pragma once

#include "Common.h"
#include "WindowsMin.h"

#include <tuple>


namespace Illulu
{
    class Input;

    class Window
    {
    public:

        Window(Input& input);
        ~Window();

        void OnInitialize() noexcept;
        void Show() const noexcept;
        void OnUpdate() noexcept;
        bool ShouldClose() const noexcept;
        // todo: change the return type
        std::pair<i32, i32> GetClientSize() const noexcept;
        HWND GetNativeWindowHandle() const noexcept;

    private:

        LRESULT _HandleMessages(u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept;
        static LRESULT _callback_WindowProc(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept;
        static void _RegisterWindowClass() noexcept;

    private:

        HWND m_hWnd        = nullptr;
        i32  m_width       = 0;
        i32  m_height      = 0;
        bool m_minimized   = false;
        bool m_focused     = false;
        bool m_shouldClose = false;

        Input& m_input;

        static constexpr const tchar* WINDOW_CLASS_NAME = ILL_TEXT("IlluluWndClass");
    };
}
