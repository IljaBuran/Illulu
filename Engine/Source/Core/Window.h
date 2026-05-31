#pragma once

#include "Common.h"
#include "WindowsMin.h"

namespace Illulu
{
    class Window
    {
    public:

        Window() = default;
        ~Window();

        void Initialize();

    private:

        static LRESULT WindowProc(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept;
        static WNDCLASSEX RegisterWindowClass();

    private:

        HWND m_hWnd   = nullptr;

        static constexpr const wchar_t* WINDOW_CLASS_NAME = TEXT("IlluluWndClass");
        static constexpr const i32 INIT_WIDTH  = 1920;
        static constexpr const i32 INIT_HEIGHT = 1080;
    };
}
