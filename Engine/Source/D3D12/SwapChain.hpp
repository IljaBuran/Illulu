#include "Common.hpp"

#include "DX12.hpp"

#include "Array.hpp"
#include "Pair.hpp"

#include "DescriptorUtil.hpp"

namespace Illulu::D3D12
{
    class SwapChain
    {
    public:

        SwapChain() = default;

        void Create(
            IDXGIFactoryIll* const factory, 
            ID3D12DeviceIll* const device,
            ID3D12CommandQueueIll* const commandList, 
            HWND const hWnd, 
            u32 width, 
            u32 height
        );
        
        void Resize(ID3D12DeviceIll* device, i32 newWidth, i32 newHeight);
        
        Pair<u32, u32> GetSize() const
        {
            return Pair<u32, u32>{m_renderTargetWidth, m_renderTargetHeight};
        }
        
        f32 GetAspectRatio() const
        {
            return m_aspectRatio;
        }

        void UpdateBackbufferIndex()
        {
            m_backbufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();
        }

        IDXGISwapChainIll* GetSwapChainPtr()
        {
            ILL_ASSERT(m_dxgiSwapChain);

            return m_dxgiSwapChain.Get();
        }

        IDXGISwapChainIll* operator->()
        {
            return GetSwapChainPtr();
        }

        operator IDXGISwapChainIll*()
        {
            return GetSwapChainPtr();
        }

        SwapChain(const SwapChain&) = delete;
        SwapChain(SwapChain&&) = delete;
        void operator=(const SwapChain&) = delete;
        void operator=(SwapChain&&) = delete;

        u32 m_backbufferIndex{};
        Array<ComPtr<ID3D12Resource>, FRAMEBUFFER_COUNT> m_buffers{};
        DescriptorHeap            m_rtvHeap{};

    private:

        void _UpdateAspectRatio();

    private:

        ComPtr<IDXGISwapChainIll> m_dxgiSwapChain{};

        u32 m_renderTargetWidth{};
        u32 m_renderTargetHeight{};
        f32 m_aspectRatio{};
    };
}