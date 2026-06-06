#include "Engine.h"

namespace Illulu
{
    Engine::Engine() noexcept
        : m_window(m_input)
    {
    }

    void Engine::Run()
    {
        Initialize();

        while (!m_window.ShouldClose())
        {
            Update();
        }
    }

    void Engine::Initialize()
    {
        m_window.OnInitialize();
        HWND hWnd = m_window.GetNativeWindowHandle();
        assert(hWnd);
        auto [width, height] = m_window.GetClientSize();
        m_renderer.OnInitialize(hWnd, width, height);


        m_window.Show();
    }

    void Engine::Update() noexcept
    {
        m_input.OnUpdate();
        m_window.OnUpdate();

        Sleep(1000 / 60);
    }
}

