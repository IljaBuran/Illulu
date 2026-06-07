#pragma once

#include "Common.h"

#include <vector>
#include <array>

#include "DX12.h"

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

#include "RHI/DescriptorUtil.h"
#include "RHI/InfoQueue.h"

namespace Illulu
{
    class Renderer
    {
    public: /* Public functions */
    
        void OnInitialize(HWND hWnd, i32 width, i32 height);
        void OnUpdate();
        void OnShutdown();

        void ResizeBackbuffer();

    private: /* Private functions */

        void _FlushCommandQueue();
        ID3D12Resource* _GetCurrentBackbuffer() noexcept;

        void _CreateCommandObjects() ;
        void _CreateSwapChain(HWND hWnd, i32 width, i32 height);
        void _CreateRTVAndDSVDescriptorHeaps() noexcept;

        CD3DX12_CPU_DESCRIPTOR_HANDLE _GetCurrentBackbufferView() noexcept;
        CD3DX12_CPU_DESCRIPTOR_HANDLE _GetDepthStencilView() noexcept;

        // imgui
        void _ImGuiInit(HWND hWnd);
        void _ImGuiBeginFrame();
        void _ImGuiEndFrame();
        void _ImGuiDestroy();


    private: /* Constants */

        static constexpr D3D_FEATURE_LEVEL FEATURE_LEVEL             = D3D_FEATURE_LEVEL_12_2;
        static constexpr u32               SWAPCHAIN_BUFFER_COUNT    = 2;
        static constexpr DXGI_FORMAT       DEPTH_STENCIL_FORMAT      = DXGI_FORMAT_D24_UNORM_S8_UINT;
        static constexpr u32               CBV_SRV_UAV_HEAP_CAPACITY = 16384;

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
        CbvSrvUavHeap                        m_cbvSrvUavHeap;

        InfoQueue                            m_infoQueue;

        u8                                   m_currBackBuffer{};
        HANDLE                               m_fenceEvent;
        u64                                  m_currentFence{};

        u32                                  m_imguiFontSrvIndex = UINT32_MAX;
        
        ComPtr<ID3D12Resource>                                     m_depthStencilBuffer;
        std::array<ComPtr<ID3D12Resource>, SWAPCHAIN_BUFFER_COUNT> m_swapChainBuffer;

    public: /* Debug */
    
        void debug_EnableDebugLayer();
    };
}