#include "Core/Input.h"
#include "WindowsMin.h"

namespace Illulu
{
    void Input::OnUpdate() noexcept
    {
        _ResetPressedReleased();
    }

    void Input::NotifyKeyUp(keyCode key) noexcept
    {
        if (m_keysDown.test(key))
            m_keysReleased.set(key);

        m_keysDown.set(key, false);
    }

    void Input::NotifyKeyDown(keyCode key) noexcept
    {
        if (!m_keysDown.test(key))
            m_keysPressed.set(key);

        m_keysDown.set(key);
    }

    bool Input::IsKeyPressed(keyCode key) const noexcept
    {
        return m_keysPressed.test(key);
    }

    bool Input::IsKeyReleased(keyCode key) const noexcept
    {
        return m_keysReleased.test(key);
    }

    bool Input::IsKeyDown(keyCode key) const noexcept
    {
        return m_keysDown.test(key);
    }

    void Input::_ResetPressedReleased() noexcept
    {
        m_keysPressed.reset();
        m_keysReleased.reset();
    }

    void Input::debug_PrintState() const noexcept
    {
        OutputDebugString(String(L"KeysPressed:" + BitsetToKeys(m_keysPressed)).c_str());
        OutputDebugString(String(L"KeysReleased:" + BitsetToKeys(m_keysReleased)).c_str());
        OutputDebugString(String(L"KeysDown:" + BitsetToKeys(m_keysDown)).c_str());
    }
}