#include "Renderer.h"

#include "Common.h"
#include "WindowsMin.h"

namespace Illulu
{
    void Renderer::OnInitialize(HWND hWnd, i32 clientWidth, i32 clientHeight)
    {
        u32 factoryFlags = 0;

    //#if defined(_DEBUG)
    #if 1
        factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
        debug_EnableDebugLayer();
    #endif

        // create DXGIFactory
        WIN_CHECK(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_DXGIFactory)));

        // create device through adapters
        ComPtr<IDXGIAdapter1> adapterV1;

        // method to find device which supports feature_level
        bool found = false;
        for (i32 i = 0; m_DXGIFactory->EnumAdapters1(i, adapterV1.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; i++)
        {
            HRESULT hRes = D3D12CreateDevice(adapterV1.Get(), FEATURE_LEVEL, IID_PPV_ARGS(&m_device));

            if (SUCCEEDED(hRes))
            {
                found = true;
                break;
            }
        }
        assert(found);

        // create fence throught newly created device
        WIN_CHECK(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

        // create command queue, allocator, list
        _CreateCommandObjects();

        // reopen command list for initialization commands
        WIN_CHECK(m_directCommandListAllocator->Reset());
        WIN_CHECK(m_commandList->Reset(m_directCommandListAllocator.Get(), nullptr));

        // create swapchain
        _CreateSwapChain(hWnd, clientWidth, clientHeight);

        // create RTV and DSV descriptor heaps
        _CreateRTVAndDSVDescriptorHeaps();

        // create render target view
        for (u32 i = 0; i < SWAPCHAIN_BUFFER_COUNT; i++)
        {
            WIN_CHECK(m_DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapChainBuffer[i])));
            m_device->CreateRenderTargetView(m_swapChainBuffer[i].Get(), nullptr, m_RTVHeap.GetCpuHandle(i));
        }

        // create depth stencil view
        D3D12_RESOURCE_DESC depthStencilDesc
        {
            .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            .Alignment = 0,
            .Width = static_cast<u64>(clientWidth),
            .Height = static_cast<u64>(clientHeight),
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DEPTH_STENCIL_FORMAT,
            .SampleDesc
            {
                    .Count = 1,
                    .Quality = 0
        },
            .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
            .Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        };

        D3D12_CLEAR_VALUE optClear
        {
            .Format = DEPTH_STENCIL_FORMAT,
            .DepthStencil
            {
                    .Depth = 1.0f,
                    .Stencil = 0
        }
        };

        const CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        WIN_CHECK(m_device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &depthStencilDesc,
            D3D12_RESOURCE_STATE_COMMON,
            &optClear,
            IID_PPV_ARGS(&m_depthStencilBuffer)));

        m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, GetDepthStencilView());

        const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                m_depthStencilBuffer.Get(),
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_DEPTH_WRITE);

        m_commandList->ResourceBarrier(
            1,
            &barrier
        );

        // create viewport
        D3D12_VIEWPORT viewportDesc
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<f32>(clientWidth),
            .Height = static_cast<f32>(clientHeight),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };

        m_commandList->RSSetViewports(1, &viewportDesc);
    }

    void Renderer::OnUpdate() noexcept
    {
    }

    std::vector<DXGI_MODE_DESC1> Renderer::_GetDisplayModes(const ComPtr<IDXGIOutput6>& output6) const
    {
        u32 count{};
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

        // get count of modelists
        WIN_CHECK(output6->GetDisplayModeList1(format, 0, &count, nullptr));

        // get the modelists
        std::vector<DXGI_MODE_DESC1> modeLists(count);
        WIN_CHECK(output6->GetDisplayModeList1(format, 0, &count, &modeLists[0]));

        return modeLists;
    }

    void Renderer::_CreateCommandObjects()
    {
        // describe command queue and create one
        D3D12_COMMAND_QUEUE_DESC queueDesc
        {
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
            .NodeMask = 0
        };
        WIN_CHECK(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

        // create direct commandlist allocator
        WIN_CHECK(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_directCommandListAllocator)));

        // create commandlist
        WIN_CHECK(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_directCommandListAllocator.Get(), nullptr,
            IID_PPV_ARGS(&m_commandList)));

        m_commandList->Close();
    }

    void Renderer::_CreateSwapChain(HWND hWnd, i32 width, i32 height)
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc
        {
            .Width = static_cast<u32>(width),
            .Height = static_cast<u32>(height),
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .Stereo = false,
            .SampleDesc
            {
                    .Count = 1,
                    .Quality = 0
            },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = SWAPCHAIN_BUFFER_COUNT,
            .Scaling = DXGI_SCALING_NONE,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
            .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
        };

        ComPtr<IDXGISwapChain1> swapChainV1;

        WIN_CHECK(m_DXGIFactory->CreateSwapChainForHwnd(m_commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, swapChainV1.GetAddressOf()));

        WIN_CHECK(swapChainV1.As<IDXGISwapChainIll>(&m_DXGISwapChain));
    }

    void Renderer::_CreateRTVAndDSVDescriptorHeaps() noexcept
    {
        m_RTVHeap.Init(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SWAPCHAIN_BUFFER_COUNT);
        m_DSVHeap.Init(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE Renderer::GetCurrentBackBufferView() noexcept
    {
        return m_RTVHeap.GetCpuHandle(m_currBackBuffer);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE Renderer::GetDepthStencilView() noexcept
    {
        return m_DSVHeap.GetCpuHandle(0);
    }

    void Renderer::debug_LogAdaptersAndOutputs()
    {
        HRESULT hRes;

        u32 i = 0;

        ComPtr<IDXGIAdapter1> adapterV1;

        while ((hRes = m_DXGIFactory->EnumAdapters1(i, adapterV1.ReleaseAndGetAddressOf())) != DXGI_ERROR_NOT_FOUND)
        {
            ComPtr<IDXGIAdapterIll> adapter;

            WIN_CHECK(adapterV1.As<IDXGIAdapterIll>(&adapter));

            DXGI_ADAPTER_DESC3 adapterDesc;
            adapter->GetDesc3(&adapterDesc);

            // here it won't compile if UNICODE not used, adapterDesc.Description is hardcoded wchar
            String adapterString = std::format(L"Adapter: {}\n", adapterDesc.Description);
            OutputDebugString(adapterString.c_str());

            u32 j = 0;
            ComPtr<IDXGIOutput> outputV0;
            while ((hRes = adapter->EnumOutputs(j, outputV0.ReleaseAndGetAddressOf())) == S_OK)
            {
                ComPtr<IDXGIOutputIll> output;
                WIN_CHECK(outputV0.As<IDXGIOutputIll>(&output));

                DXGI_OUTPUT_DESC1 outputDesc;
                WIN_CHECK(output->GetDesc1(&outputDesc));

                // here it won't compile if UNICODE not used, outputDesc.DeviceName is hardcoded wchar
                String outputString = std::format(L"\tMonitor: {} \n", outputDesc.DeviceName);
                OutputDebugString(outputString.c_str());

                std::vector<DXGI_MODE_DESC1> modeLists = _GetDisplayModes(output);

                for (const auto& x : modeLists)
                {
                    f32 nom = static_cast<f32>(x.RefreshRate.Numerator);
                    f32 denom = static_cast<f32>(x.RefreshRate.Denominator);
                    f32 refreshRate = nom / denom;

                    u32 width = x.Width;
                    u32 height = x.Height;

                    OutputDebugString(std::format(L"\t\t{}x{}@{}Hz\n", width, height, refreshRate).c_str());
                }
                j++;
            }
            assert(hRes == DXGI_ERROR_NOT_FOUND);

            i++;
        }

        assert(hRes == DXGI_ERROR_NOT_FOUND);

    }

    void Renderer::debug_EnableDebugLayer()
    {
        ComPtr<ID3D12DebugIll> debugController;
        WIN_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));

        debugController->EnableDebugLayer();
        debugController->SetEnableGPUBasedValidation(true);
    }

    void DescriptorHeap::Init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE descHeapType, u32 capacity)
    {
        assert(!m_heap);
        assert(device);

        D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc
        {
            .Type = descHeapType,
            .NumDescriptors = capacity,
            .Flags = (descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) ?
            D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = 0
        };

        WIN_CHECK(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(m_heap.GetAddressOf())));

        m_descriptorSize = device->GetDescriptorHandleIncrementSize(descHeapType);
    }

    ID3D12DescriptorHeap* DescriptorHeap::GetHeap() const noexcept
    {
        return m_heap.Get();
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCpuHandle(u32 index) const noexcept
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU(m_heap->GetCPUDescriptorHandleForHeapStart());

        hCPU.Offset(index, m_descriptorSize);
        return hCPU;
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGpuHandle(u32 index) const noexcept
    {
        CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU(m_heap->GetGPUDescriptorHandleForHeapStart());

        hGPU.Offset(index, m_descriptorSize);
        return hGPU;
    }

}
