#pragma once

#include "Common/Common.h"
#include <bitset>

class Input
{
public:

    void NotifyKeyUp(u32 key) noexcept;
    void NotifyKeyDown(u32 key) noexcept;
    
    bool IsKeyPressed(u32 key) const noexcept;
    bool IsKeyReleased(u32 key) const noexcept;
    bool IsKeyDown(u32 key) const noexcept;

private:

    static constexpr u32 KEY_COUNT = 256;

    std::bitset<KEY_COUNT> m_keysPressed;
    std::bitset<KEY_COUNT> m_keysReleased;
    std::bitset<KEY_COUNT> m_keysDown;
};