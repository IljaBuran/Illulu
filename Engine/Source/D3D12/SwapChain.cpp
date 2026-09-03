#include "SwapChain.hpp"

namespace Illulu::D3D12
{
    void SwapChain::Create(
        IDXGIFactoryIll* const factory,
        ID3D12DeviceIll* const device,
        ID3D12CommandQueueIll* const commandQueue,
        HWND const hWnd,
        u32 width,
        u32 height
    )
    {
        m_renderTargetWidth = width;
        m_renderTargetHeight = height;
        
        _UpdateAspectRatio();

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc
        {
            .Width{width},
            .Height{height},
            .Format{DXGI_FORMAT_R8G8B8A8_UNORM},
            .Stereo{false},
            .SampleDesc
            {
                .Count{1},
                .Quality{0}
            },
            .BufferUsage{DXGI_USAGE_RENDER_TARGET_OUTPUT},
            .BufferCount{FRAMEBUFFER_COUNT},
            .Scaling{DXGI_SCALING_STRETCH},
            .SwapEffect{DXGI_SWAP_EFFECT_FLIP_DISCARD},
            .AlphaMode{DXGI_ALPHA_MODE_UNSPECIFIED},
            .Flags{0}
        };

        ComPtr<IDXGISwapChain1> swapChainV1;

        WIN_CHECK(factory->CreateSwapChainForHwnd(
            commandQueue,
            hWnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            swapChainV1.GetAddressOf()
        ));

        WIN_CHECK(swapChainV1.As<IDXGISwapChainIll>(&m_dxgiSwapChain));

        // TODO: decide whether I want to react to alt-enter
        /* Disable Alt-Enter fullscreen toggling (we'll be implementing it ourselves) */
        WIN_CHECK(factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

        m_backbufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

        /* Create RTV's descriptor heap */
        m_rtvHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FRAMEBUFFER_COUNT);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap.GetCpuHandle(0));

        // for each frame create rtv
        for (u32 i{0}; i < FRAMEBUFFER_COUNT; i++)
        {
            WIN_CHECK(m_dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_buffers[i])));
            device->CreateRenderTargetView(m_buffers[i].Get(), nullptr, rtvHandle);

            rtvHandle.Offset(1, m_rtvHeap.GetDescSize());
        }
    }
    
    // TODO: IMPLEMENT
        // don't forget to call _UpdateAspectRatio()
    void SwapChain::Resize(ID3D12DeviceIll* const device, i32 newWidth, i32 newHeight)
    {
        m_renderTargetWidth = newWidth;
        m_renderTargetHeight = newHeight;

        // release previous backbuffers
        for (u32 i{0}; i < FRAMEBUFFER_COUNT; i++)
        {
            m_buffers[i].Reset();
        }

        // extract previous settings
        DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
        WIN_CHECK(m_dxgiSwapChain->GetDesc1(&swapchainDesc));

        WIN_CHECK(m_dxgiSwapChain->ResizeBuffers(
            FRAMEBUFFER_COUNT,
            m_renderTargetWidth,
            m_renderTargetHeight,
            swapchainDesc.Format,
            swapchainDesc.Flags
        ));

        // update backbuffer index
        m_backbufferIndex = static_cast<u8>(m_dxgiSwapChain->GetCurrentBackBufferIndex());

        // bind buffers and create rtvs
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap.GetCpuHandle(0));
        for (u32 i{0}; i < FRAMEBUFFER_COUNT; i++)
        {
            WIN_CHECK(m_dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_buffers[i])));
            device->CreateRenderTargetView(m_buffers[i].Get(), nullptr, rtvHandle);

            rtvHandle.Offset(1, m_rtvHeap.GetDescSize());
        }
    }

    void SwapChain::_UpdateAspectRatio()
    {
        ILL_ASSERT(m_renderTargetHeight > 0);
        
        m_aspectRatio = static_cast<f32>(m_renderTargetWidth) / static_cast<f32>(m_renderTargetHeight);
    }
}

