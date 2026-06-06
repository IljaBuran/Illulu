#pragma once

#include "Common.h"
#include <wrl/client.h>

#include <D3dx12.h>

#include <d3d12sdklayers.h>
#include <dxgi1_6.h>

#include <vector>
#include <array>


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

    class DescriptorHeap
    {
    public:

        DescriptorHeap() = default;

        void Init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE desc_heap_type, u32 capacity);

        ID3D12DescriptorHeap* GetHeap() const noexcept;

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(u32 index) const noexcept;
        CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(u32 index) const noexcept;

    private:
        ComPtr<ID3D12DescriptorHeap> m_heap;
        u32 m_descriptorSize = 0;
    };

    class Renderer
    {
    public: /* Public functions */
    
        void OnInitialize(HWND hWnd, i32 width, i32 height);
        void OnUpdate() noexcept;

    private: /* Private functions */

        std::vector<DXGI_MODE_DESC1> _GetDisplayModes(const ComPtr<IDXGIOutputIll>& output) const;
        void _CreateCommandObjects() ;
        void _CreateSwapChain(HWND hWnd, i32 width, i32 height);
        void _CreateRTVAndDSVDescriptorHeaps() noexcept;
        CD3DX12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView() noexcept;
        CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() noexcept;

    private: /* Constants */

        static constexpr D3D_FEATURE_LEVEL FEATURE_LEVEL          = D3D_FEATURE_LEVEL_12_2;
        static constexpr u32               SWAPCHAIN_BUFFER_COUNT = 2;
        static constexpr DXGI_FORMAT       DEPTH_STENCIL_FORMAT   = DXGI_FORMAT_D24_UNORM_S8_UINT;

    private: /* Private variables */

        ComPtr<IDXGIFactoryIll>              m_DXGIFactory;
        ComPtr<IDXGISwapChainIll>            m_DXGISwapChain;

        ComPtr<ID3D12DeviceIll>              m_device;
        ComPtr<ID3D12FenceIll>               m_fence;
        ComPtr<ID3D12CommandQueueIll>        m_commandQueue;
        ComPtr<ID3D12CommandAllocator>       m_directCommandListAllocator;
        ComPtr<ID3D12GraphicsCommandListIll> m_commandList;

        DescriptorHeap                       m_RTVHeap;
        DescriptorHeap                       m_DSVHeap;

        u8                                   m_currBackBuffer{};
        
        ComPtr<ID3D12Resource>                                     m_depthStencilBuffer;
        std::array<ComPtr<ID3D12Resource>, SWAPCHAIN_BUFFER_COUNT> m_swapChainBuffer;

    public: /* Debug */
    
        void debug_LogAdaptersAndOutputs();
        void debug_EnableDebugLayer();
    };
}