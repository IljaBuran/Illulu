#include "Engine.h"

namespace Illulu
{
    Engine::Engine() noexcept
        : m_window(m_input)
    {
    }

    void Engine::Run()
    {
        _Initialize();

        while (!m_window.ShouldClose())
        {
            _Update();
        }
    }

    void Engine::_Initialize()
    {
        _ConfigureDelegates();
        
        m_window.OnInitialize();
        HWND hWnd = m_window.GetNativeWindowHandle();

        m_renderer.OnInitialize(hWnd);

        m_window.Show();
    }

    void Engine::_Update()
    {
        m_timer.Tick();
        
        m_input.OnUpdate();
        m_window.OnUpdate();

        m_renderer.OnUpdate();

        m_renderer.OnRender();
    }
    
    void Engine::_Shutdown()
    {
        m_renderer.OnShutdown();
    }

    void Engine::_ConfigureDelegates()
    {
        m_window.AddResizeListener<Renderer, &Renderer::UpdateRenderTargetSize>(&m_renderer);
    }
}