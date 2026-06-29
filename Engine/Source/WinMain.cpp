#include "Common.h"

#include "WindowsMin.h"

#include "Core/Engine.h"

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 619;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

int WinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPSTR lpCmdLine,
    [[maybe_unused]] INT nCmdShow)
{
    Illulu::Engine engine;

    try
    {
        engine.Run();
    }
    catch (Illulu::ILLException e)
    {
        MessageBox(nullptr, (e.GetMessageW()).c_str(), L"Assertion failed", MB_OK);
    }

    return 0;
}