#include "Common.h"

#include "WindowsMin.h"

#include "Core/Window.h"

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 619;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

i32 WinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPSTR lpCmdLine,
    [[maybe_unused]] i32 nCmdShow)
{
    Illulu::Window win;
    win.Initialize();
    win.Show();

    while (!win.ShouldClose())
    {
        win.PullMessages();

        Sleep(1000/60);
    }

    return 0;
}