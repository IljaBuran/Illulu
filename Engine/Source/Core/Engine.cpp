#include "Engine.hpp"

#include "Core/Console.hpp"

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

            if (i32 dX = m_input.xMouseDelta, dY = m_input.yMouseDelta; dX || dY)
            {
                INFO(L"Delta:({},{})", dX, dY);
            }
        }
    }

    void Engine::_Initialize()
    {
        INFO(L"*** ENGINE INITIALIZATION START ***");
        _ConfigureDelegates();

        //FATAL(L"*** This is error message ***");
        //WARNING(L"*** This is warning message ***");
        //INFO(L"*** This is info message ***");

        m_window.OnInitialize();
        HWND hWnd = m_window.GetNativeWindowHandle();

        m_renderer.OnInitialize(hWnd);

        m_window.Show();

        INFO(L"*** ENGINE INITIALIZATION END ***");
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
        INFO(L"Delegates Configured");
    }
}