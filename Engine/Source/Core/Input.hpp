#pragma once

#include "Common.hpp"
#include "BitSet.hpp"

namespace Illulu
{
    enum class MouseButton: u8
    {
        LEFT,
        RIGHT,
        MIDDLE,
        COUNT
    };

    class Input
    {
    public:

        void OnUpdate() noexcept;

        void NotifyKeyUp(keyCode key)   noexcept;
        void NotifyKeyDown(keyCode key) noexcept;

        bool IsKeyPressed(keyCode key)  const noexcept;
        bool IsKeyReleased(keyCode key) const noexcept;
        bool IsKeyDown(keyCode key)     const noexcept;

        void NotifyMouseUp(MouseButton button)     noexcept;
        void NotifyMouseDown(MouseButton button)   noexcept;
        void NotifyNewMousePostition(i32 x, i32 y) noexcept;
        void NotifyInvalidateMousePosition()       noexcept;

        bool IsMouseBtnPressed(MouseButton button)  const noexcept;
        bool IsMouseBtnReleased(MouseButton button) const noexcept;
        bool IsMouseBtnDown(MouseButton button)     const noexcept;

        friend class Engine;

    private:

        void _ResetPressedReleased() noexcept;

    private:

        static constexpr u32 KEY_COUNT{256};
        static constexpr u32 MOUSE_BUTTON_COUNT{static_cast<u32>(MouseButton::COUNT)};

        // keyboard
        BitSet<KEY_COUNT> m_keysPressed;
        BitSet<KEY_COUNT> m_keysReleased;
        BitSet<KEY_COUNT> m_keysDown;

        // mouse
        BitSet<MOUSE_BUTTON_COUNT> m_mousePressed;
        BitSet<MOUSE_BUTTON_COUNT> m_mouseReleased;
        BitSet<MOUSE_BUTTON_COUNT> m_mouseDown;

        bool mousePosInvalid{true};
        i32 xMousePosition{};
        i32 yMousePosition{};
        i32 xMouseDelta{};
        i32 yMouseDelta{};

    public:

        void debug_PrintState() const noexcept;
    };
}