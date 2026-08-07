#pragma once

#include "Common.hpp"
#include "WindowsMin.hpp"

// todo
#include <tuple>
#include "Util/Delegate.hpp"

namespace Illulu
{
    class Input;

    class Window
    {
    public: /* Public Functions */

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

        template<typename T, void (T::* Method)(i32, i32)>
        void AddResizeListener(T* object)
        {
            m_resizeDelegate.Add<T, Method>(object);
        }

    private: /* Private Functions */

        LRESULT _HandleMessages(u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept;
        static LRESULT _callback_WindowProc(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam) noexcept;
        static void _RegisterWindowClass() noexcept;

    private: /* Variables */

        HWND m_hWnd{nullptr};
        i32  m_width{0};
        i32  m_height{0};
        bool m_minimized{false};
        bool m_maximized{false};
        bool m_fullscreenMode{false};
        bool m_focused{false};
        bool m_shouldClose{false};

        Input& m_input;
        
        MulticastDelegate<i32, i32> m_resizeDelegate;


        static constexpr const wchar* WINDOW_CLASS_NAME = L"IlluluWndClass";
    };
}
