#include "Engine.h"

Illulu::Engine::Engine() noexcept
    : m_window(m_input)
{
}

void Illulu::Engine::Run() noexcept
{
    Initialize();
    
    while (!m_window.ShouldClose())
    {
        Update();
    }
}

void Illulu::Engine::Initialize() noexcept
{
    m_window.OnInitialize();
    HWND hWnd = m_window.GetNativeWindowHandle();
    auto [width, height] = m_window.GetClientSize();
    m_renderer.OnInitialize(hWnd, width, height);


    m_window.Show();
}

void Illulu::Engine::Update() noexcept
{
    m_input.OnUpdate();
    m_window.OnUpdate();

    Sleep(1000 / 60);
}