#include "Core/Input.hpp"
#include "WindowsMin.hpp"

#include "String.hpp"

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

    void Input::NotifyMouseUp(MouseButton button) noexcept
    {
        ILL_ASSERT(button < MouseButton::COUNT);

        u64 _button{static_cast<u64>(button)};
        if (m_mouseDown.test(_button))
            m_mouseReleased.set(_button);

        m_mouseDown.set(_button, false);
    }

    void Input::NotifyMouseDown(MouseButton button) noexcept
    {
        ILL_ASSERT(button < MouseButton::COUNT);

        u64 _button{static_cast<u64>(button)};
        if (!m_mouseDown.test(_button))
            m_mousePressed.set(_button);

        m_mouseDown.set(_button);
    }

    void Input::NotifyNewMousePostition(i32 x, i32 y) noexcept
    {
        // note: if invalidated (coming back from unfocused, etc...) we set delta to 0.0f
        xMouseDelta = mousePosInvalid ? 0 : x - xMousePosition;
        yMouseDelta = mousePosInvalid ? 0 : y - yMousePosition;

        mousePosInvalid = false;

        xMousePosition = x;
        yMousePosition = y;
    }

    void Input::NotifyInvalidateMousePosition() noexcept
    {
        mousePosInvalid = true;
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

    bool Input::IsMouseBtnPressed(MouseButton button) const noexcept
    {
        u64 _key{static_cast<u64>(button)};
        return m_mousePressed.test(_key);
    }

    bool Input::IsMouseBtnReleased(MouseButton button) const noexcept
    {
        u64 _key{static_cast<u64>(button)};
        return m_mouseReleased.test(_key);
    }

    bool Input::IsMouseBtnDown(MouseButton button) const noexcept
    {
        u64 _key{static_cast<u64>(button)};
        return m_mouseDown.test(_key);
    }

    void Input::_ResetPressedReleased() noexcept
    {
        m_keysPressed.reset();
        m_keysReleased.reset();
    }

    void Input::debug_PrintState() const noexcept
    {
        INFO(L"KeysPressed:", BitsetToKeys(m_keysPressed));
        INFO(L"KeysReleased: {}", BitsetToKeys(m_keysReleased));
        INFO(L"KeysDown: {}", BitsetToKeys(m_keysDown));
    }
}