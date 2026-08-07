#pragma once

#include "Common.hpp"

#include "Core/Input.hpp"
#include "Core/Window.hpp"
#include "Core/Renderer.hpp"
#include "Core/Timer.hpp"

#include "Core/Console.hpp"

namespace Illulu
{
    class Engine
    {
    public:

        Engine() noexcept;
        void Run();

    private: /* private functions */
        
        void _Initialize();
        void _Update();
        void _Shutdown();

        void _ConfigureDelegates();

    private: /* private variables */

        Input    m_input;
        Window   m_window;
        Renderer m_renderer;
        Timer    m_timer;
    };
}