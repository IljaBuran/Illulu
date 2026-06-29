#pragma once

#include "Common.h"

#include <vector>
#include "Array.h"

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
    
        void OnInitialize(HWND hWnd);
        void OnUpdate();
        void OnRender();
        void OnShutdown();

        void UpdateRenderTargetSize(i32 newWidth, i32 newHeight);

    private: /* Private functions */

        f32 _GetRenderTargetAspectRatio() const;
        
        void _FeedCommandList();
        void _WaitForGpu();
        void _MoveToNextFrame();
        ID3D12Resource* _GetCurrentBackbuffer() noexcept;

    private: /* Private variables */

        /* pipeline objects */
        ComPtr<IDXGIFactoryIll>                        m_factory{};

        ComPtr<IDXGIAdapterIll>                        m_adapter{};
        ComPtr<ID3D12DeviceIll>                        m_device{};

        ComPtr<IDXGISwapChainIll>                      m_swapchain{};
        D3D12_VIEWPORT                                 m_viewport{};
        D3D12_RECT                                     m_scissorRect{};

        Array<ComPtr<ID3D12Resource>, FRAMEBUFFER_COUNT>         m_renderTargets{};
        Array<ComPtr<ID3D12CommandAllocator>, FRAMEBUFFER_COUNT> m_commandListAllocators{};

        ComPtr<ID3D12CommandQueueIll>                  m_commandQueue{};
        ComPtr<ID3D12GraphicsCommandListIll>           m_commandList{};
                                 
        ComPtr<ID3D12RootSignature>                    m_rootSignature{};
        ComPtr<ID3D12PipelineState>                    m_pipelineState{};

        u32                                            m_rtvDescriptorSize{};

        InfoQueue                                      m_infoQueue{};

        ComPtr<ID3D12DescriptorHeap> m_rtvHeap{};

        /* synchronization */
        u8                          m_frameIndex{};
        ComPtr<ID3D12FenceIll>      m_fence{};
        Event                       m_fenceEvent{};
        Array<u64, FRAMEBUFFER_COUNT> m_fenceValues{};

        /* app resources */
        ComPtr<ID3D12Resource>   m_vertexBuffer{};  
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};

        /* render target info */ 
        i32                      m_renderTargetWidth{};
        i32                      m_renderTargetHeight{};

        bool m_initialized{false};
    };
}