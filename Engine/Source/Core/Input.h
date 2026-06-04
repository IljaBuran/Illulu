#pragma once

#include "Common.h"
#include <bitset>

namespace Illulu
{
    class Input
    {
    public:

        void OnUpdate() noexcept;
        
        void NotifyKeyUp(keyCode key) noexcept;
        void NotifyKeyDown(keyCode key) noexcept;
    
        bool IsKeyPressed(keyCode key) const noexcept;
        bool IsKeyReleased(keyCode key) const noexcept;
        bool IsKeyDown(keyCode key) const noexcept;

    private:

        void _ResetPressedReleased() noexcept;
        
    private:
        static constexpr u32 KEY_COUNT = 256;

        std::bitset<KEY_COUNT> m_keysPressed;
        std::bitset<KEY_COUNT> m_keysReleased;
        std::bitset<KEY_COUNT> m_keysDown;
    
    public:

        void debug_PrintState() const noexcept;
    };
}