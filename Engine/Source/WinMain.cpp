#include "Common.hpp"

#include "WindowsMin.hpp"

#include "Core/Engine.hpp"

extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 619;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

int WinMain(
    UNUSED HINSTANCE hInstance,
    UNUSED HINSTANCE hPrevInstance,
    UNUSED LPSTR lpCmdLine,
    UNUSED INT nCmdShow
)
{
    Illulu::Engine engine;

    try
    {
        engine.Run();
    }
    catch (Illulu::ILLException e)
    {
        MessageBox(nullptr, (e.GetErrorMessage()).c_str(), L"Assertion failed", MB_OK);
    }

    return 0;
}