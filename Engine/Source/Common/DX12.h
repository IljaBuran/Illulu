#include <wrl/client.h>

#include <D3dx12.h>

#include <d3d12sdklayers.h>
#include <dxgi1_6.h>

namespace Illulu
{
    using Microsoft::WRL::ComPtr;

    using IDXGIAdapterIll = IDXGIAdapter4;
    using IDXGIOutputIll = IDXGIOutput6;
    using IDXGIFactoryIll = IDXGIFactory7;
    using IDXGISwapChainIll = IDXGISwapChain4;

    using ID3D12DeviceIll = ID3D12Device15;
    using ID3D12DebugIll = ID3D12Debug6;
    using ID3D12FenceIll = ID3D12Fence1;
    using ID3D12CommandQueueIll = ID3D12CommandQueue1;
    using ID3D12GraphicsCommandListIll = ID3D12GraphicsCommandList10;
    using ID3D12InfoQueueIll = ID3D12InfoQueue1;
}