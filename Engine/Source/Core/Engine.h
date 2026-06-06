#pragma once

#include "Common.h"

#include "Core/Input.h"
#include "Core/Window.h"
#include "Core/Renderer.h"

namespace Illulu
{
    class Engine
    {
    public:

        Engine() noexcept;
        void Run();

    private:
        
        void Initialize();
        void Update() noexcept;

    private:

        Input    m_input;
        Window   m_window;
        Renderer m_renderer;
    };
}


