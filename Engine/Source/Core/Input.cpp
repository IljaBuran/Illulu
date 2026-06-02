#include "Core/Input.h"

void Input::NotifyKeyUp(u32 key) noexcept
{
    ILL_ASSERT(ILL_TEXT("Invalid key"), key < 0xFF);
    
    if (m_keysDown.test(key))
        m_keysReleased.set(key);

    m_keysDown.set(key, false);
}

void Input::NotifyKeyDown(u32 key) noexcept
{
    ILL_ASSERT(ILL_TEXT("Invalid key"), key < 0xFF);
    
    if (!m_keysDown.test(key))
        m_keysPressed.set(key);
    
    m_keysDown.set(key);
}

bool Input::IsKeyPressed(u32 key) const noexcept
{
    ILL_ASSERT(ILL_TEXT("Invalid key"), key < 0xFF);
    
    return m_keysPressed.test(key);
}

bool Input::IsKeyReleased(u32 key) const noexcept
{
    ILL_ASSERT(ILL_TEXT("Invalid key"), key < 0xFF);

    return m_keysReleased.test(key);
}

bool Input::IsKeyDown(u32 key) const noexcept
{
    ILL_ASSERT(ILL_TEXT("Invalid key"), key < 0xFF);

    return m_keysDown.test(key);
}
