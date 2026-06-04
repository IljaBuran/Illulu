#pragma once

#include "Common.h"

#include <dxgi1_6.h>
#include <wrl/client.h>
#include <d3d12sdklayers.h>

#include <vector>


namespace Illulu
{
    using Microsoft::WRL::ComPtr;
    
    using IDXGIAdapterIll   = IDXGIAdapter4;
    using IDXGIOutputIll    = IDXGIOutput6;
    using IDXGIFactoryIll   = IDXGIFactory7;
    using IDXGISwapChainIll = IDXGISwapChain4;

    using ID3D12DeviceIll              = ID3D12Device15;
    using ID3D12DebugIll               = ID3D12Debug6;
    using ID3D12FenceIll               = ID3D12Fence1;
    using ID3D12CommandQueueIll        = ID3D12CommandQueue1;
    using ID3D12GraphicsCommandListIll = ID3D12GraphicsCommandList10;

    class Renderer
    {
    
    public:
    
        void OnInitialize() noexcept;
        void OnUpdate() noexcept;

    private:

        std::vector<DXGI_MODE_DESC1> _GetDisplayModes(const ComPtr<IDXGIOutputIll>& output) const noexcept;
        void _CreateCommandObjects() noexcept;
        void _CreateSwapChain() noexcept;

    private:

        ComPtr<IDXGIFactoryIll>                   m_DXGIFactory;
        ComPtr<IDXGISwapChainIll>                 m_DXGISwapChain;
        ComPtr<ID3D12DeviceIll>                   m_Device;
        ComPtr<ID3D12FenceIll>                    m_Fence;
        ComPtr<ID3D12CommandQueueIll>             m_commandQueue;
        ComPtr<ID3D12CommandAllocator>            m_directCommandListAllocator;
        ComPtr<ID3D12GraphicsCommandListIll>      m_commandList;

        static constexpr D3D_FEATURE_LEVEL FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_2;

    public:
    
        void debug_LogAdaptersAndOutputs() noexcept;
        void debug_EnableDebugLayer() noexcept;
    };
}