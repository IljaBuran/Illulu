#pragma once

#include "Common.hpp"

#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

#include <d3dx12.h>

#include <d3d12sdklayers.h>
#include <dxgi1_6.h>

#include <dxcapi.h>

#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "DirectXTK12.lib")
#pragma comment(lib, "dxil.lib")

namespace Illulu
{
    using Microsoft::WRL::ComPtr;
    using Microsoft::WRL::Wrappers::Event;

    using IDXGIAdapterIll = IDXGIAdapter4;
    using IDXGIFactoryIll = IDXGIFactory6;
    using IDXGISwapChainIll = IDXGISwapChain3;

    using ID3D12DeviceIll = ID3D12Device;
    using ID3D12DebugIll = ID3D12Debug1;
    using ID3D12FenceIll = ID3D12Fence;
    using ID3D12CommandQueueIll = ID3D12CommandQueue;
    using ID3D12GraphicsCommandListIll = ID3D12GraphicsCommandList;
    using ID3D12InfoQueueIll = ID3D12InfoQueue1;

    static constexpr D3D_FEATURE_LEVEL D3D12_REQUIRED_FEATURE_LEVEL{D3D_FEATURE_LEVEL_12_2};
    static constexpr DXGI_FORMAT       DEPTH_STENCIL_FORMAT{DXGI_FORMAT_D24_UNORM_S8_UINT};
    static constexpr u32               FRAMEBUFFER_COUNT{2};
    static constexpr u32               CBV_SRV_UAV_HEAP_CAPACITY{64};
}

#include <DirectXPackedVector.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include "Array.hpp"

namespace Illulu
{
    struct ColorVertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;

        static constexpr
        const D3D12_INPUT_LAYOUT_DESC& GetInputLayoutDesc()
        {
            static constexpr Array<D3D12_INPUT_ELEMENT_DESC, 2> elementDesc
            {
                {
                    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ColorVertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(ColorVertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
                }
            };

            static constexpr D3D12_INPUT_LAYOUT_DESC inputDesc
            {
                .pInputElementDescs{elementDesc.data()},
                .NumElements{static_cast<u32>(elementDesc.size())}
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

        static constexpr
        const D3D12_INPUT_LAYOUT_DESC& GetInputLayoutDesc()
        {
            static constexpr Array<D3D12_INPUT_ELEMENT_DESC, 4> elementDesc
            {
                {
                    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex2, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex2, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex2, tex0), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"TEX", 1, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex2, tex1), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
                }
            };

            static constexpr D3D12_INPUT_LAYOUT_DESC inputDesc
            {
                .pInputElementDescs{elementDesc.data()},
                .NumElements{static_cast<u32>(elementDesc.size())}
            };

            return inputDesc;
        }
    };
}