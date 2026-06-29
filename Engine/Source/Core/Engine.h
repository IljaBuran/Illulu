#pragma once

#include "Common.h"

#include "Core/Input.h"
#include "Core/Window.h"
#include "Core/Renderer.h"
#include "Core/Timer.h"

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