#pragma once

#include "Common.h"

#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

#include <d3dx12.h>

#include <d3d12sdklayers.h>
#include <dxgi1_6.h>

namespace Illulu
{
    using Microsoft::WRL::ComPtr;
    using Microsoft::WRL::Wrappers::Event;

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

    static constexpr D3D_FEATURE_LEVEL FEATURE_LEVEL { D3D_FEATURE_LEVEL_12_2 };
    static constexpr DXGI_FORMAT       DEPTH_STENCIL_FORMAT { DXGI_FORMAT_D24_UNORM_S8_UINT };
    static constexpr u32               FRAMEBUFFER_COUNT { 2 };
    static constexpr u32               CBV_SRV_UAV_HEAP_CAPACITY { 16'384 };

    static constexpr bool              DISABLE_FULLSCREEN { true };
}

#include <DirectXPackedVector.h>
#include <DirectXMath.h>

#include <array> 

namespace Illulu
{
    struct Vertex1
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;

        static constexpr const D3D12_INPUT_LAYOUT_DESC& GetInputLayoutDesc()
        {
            static constexpr std::array<D3D12_INPUT_ELEMENT_DESC, 2> elementDesc =
            {
                {
                    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex1, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex1, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
                }
            };

            static constexpr D3D12_INPUT_LAYOUT_DESC inputDesc
            {
                .pInputElementDescs = elementDesc.data(),
                .NumElements = static_cast<u32>(elementDesc.size())
            };

            return inputDesc;
        }
    };

    struct Vertex2
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 tex0;
        DirectX::XMFLOAT2 tex1;

        static constexpr const D3D12_INPUT_LAYOUT_DESC& GetInputLayoutDesc()
        {
            static constexpr std::array<D3D12_INPUT_ELEMENT_DESC, 4> elementDesc =
            {
                {
                    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex2, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex2, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"TEX0", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex2, tex0), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"TEX1", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex2, tex1), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
                }
            };

            static constexpr D3D12_INPUT_LAYOUT_DESC inputDesc
            {
                .pInputElementDescs = elementDesc.data(),
                .NumElements = static_cast<u32>(elementDesc.size())
            };

            return inputDesc;
        }
    };
}