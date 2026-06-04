#include "Core/Input.h"
#include "WindowsMin.h"

void Illulu::Input::OnUpdate() noexcept
{
    _ResetPressedReleased();
}

void Illulu::Input::NotifyKeyUp(keyCode key) noexcept
{
    if (m_keysDown.test(key))
        m_keysReleased.set(key);

    m_keysDown.set(key, false);
}

void Illulu::Input::NotifyKeyDown(keyCode key) noexcept
{
    if (!m_keysDown.test(key))
        m_keysPressed.set(key);
    
    m_keysDown.set(key);
}

bool Illulu::Input::IsKeyPressed(keyCode key) const noexcept
{
    return m_keysPressed.test(key);
}

bool Illulu::Input::IsKeyReleased(keyCode key) const noexcept
{
    return m_keysReleased.test(key);
}

bool Illulu::Input::IsKeyDown(keyCode key) const noexcept
{
    return m_keysDown.test(key);
}

void Illulu::Input::_ResetPressedReleased() noexcept
{
    m_keysPressed.reset();
    m_keysReleased.reset();
}

void Illulu::Input::debug_PrintState() const noexcept
{
    OutputDebugString(string(string(ILL_TEXT("KeysPressed:")) + BitsetToKeys(m_keysPressed)).c_str());
    OutputDebugString(string(string(ILL_TEXT("KeysReleased:")) + BitsetToKeys(m_keysReleased)).c_str());
    OutputDebugString(string(string(ILL_TEXT("KeysDown:")) + BitsetToKeys(m_keysDown)).c_str());
}
