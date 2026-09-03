#include "Engine.hpp"

#include "Core/Console.hpp"

#include <memory>

namespace Illulu
{
    Engine::Engine() noexcept
        : m_window(m_input)
    {
        Logger::Add(std::make_unique<WindowsNativeConsole>());
        Logger::Add(std::make_unique<ImGUIConsole>());

        //WARNING(L"This is warning message");
        //FATAL(L"This is FATAL message");
    }

    void Engine::Run()
    {
        _Initialize();

        while (!m_window.ShouldClose())
        {
            _Update();
        }

        _Shutdown();
    }

    void Engine::_Initialize()
    {
        INFO(L"*** ENGINE INITIALIZATION START ***");
        _ConfigureDelegates();

        m_window.OnInitialize();
        HWND hWnd = m_window.GetNativeWindowHandle();

        m_renderer.OnInitialize(hWnd);

        m_window.Show();

        INFO(L"*** ENGINE INITIALIZATION END ***");
    }

    void Engine::_Update()
    {
        m_timer.Tick();
        m_window.OnUpdate();

        if (m_input.IsMouseBtnDown(MouseButton::LEFT))
        {
            m_renderer.m_deltaX = m_input.xMouseDelta;
            m_renderer.m_deltaY = m_input.yMouseDelta;
        }

        m_renderer.OnUpdate();

        // todo: make separate function for this? OnRender()?
        m_renderer.OnRender();


        // this needs to be last, it only resets the inner state
        m_input.OnUpdate();
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