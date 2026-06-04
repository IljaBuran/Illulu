#include "Renderer.h"

#include "Common.h"
#include "WindowsMin.h"


void Illulu::Renderer::OnInitialize(HWND hWnd, i32 width, i32 height) noexcept
{
    HRESULT hRes;
    u32 factoryFlags = 0;

#if defined(_DEBUG)
    factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
    debug_EnableDebugLayer();
#endif

    // create DXGIFactory
    hRes = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_DXGIFactory));
    ILL_ASSERT(ILL_TEXT("CreateDXGIFactory2 failed"), SUCCEEDED(hRes));

    // create device through adapters
    ComPtr<IDXGIAdapter1> adapterV1;

    // method to find device which supports feature_level
    bool found = false;
    for (i32 i = 0; m_DXGIFactory->EnumAdapters1(i, adapterV1.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; i++)
    {
        hRes = D3D12CreateDevice(adapterV1.Get(), FEATURE_LEVEL, IID_PPV_ARGS(&m_device));

        if (SUCCEEDED(hRes))
        {
            found = true;
            break;
        }
    }
    ILL_ASSERT(ILL_TEXT("Compatible device was not found"), found);

    // create fence throught newly created device
    hRes = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    ILL_ASSERT(ILL_TEXT("Fence creation unsuccessful"), SUCCEEDED(hRes));

    // create command queue, allocator, list
    _CreateCommandObjects();

    // create swapchain
    _CreateSwapChain(hWnd, width, height);

}

void Illulu::Renderer::OnUpdate() noexcept
{
}

std::vector<DXGI_MODE_DESC1> Illulu::Renderer::_GetDisplayModes(const ComPtr<IDXGIOutput6>& output6) const noexcept
{
    u32 count{};
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    
    // get count of modelists
    HRESULT hRes = output6->GetDisplayModeList1(format, 0, &count, nullptr);
    ILL_ASSERT(ILL_TEXT("GetDisplayModeList1 failed"), SUCCEEDED(hRes));
    
    // get the modelists
    std::vector<DXGI_MODE_DESC1> modeLists(count);
    hRes = output6->GetDisplayModeList1(format, 0, &count, &modeLists[0]);
    ILL_ASSERT(ILL_TEXT("GetDisplayModeList1 failed"), SUCCEEDED(hRes));

    return modeLists;
}

void Illulu::Renderer::_CreateCommandObjects() noexcept
{
    HRESULT hRes;

    // describe command queue and create one
    D3D12_COMMAND_QUEUE_DESC queueDesc
    {
        .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0
    };
    hRes = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
    ILL_ASSERT(ILL_TEXT("CommandQueue creation failed"), SUCCEEDED(hRes));

    // create direct commandlist allocator
    hRes = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_directCommandListAllocator));
    ILL_ASSERT(ILL_TEXT("CommandAllocator creation failed"), SUCCEEDED(hRes));

    // create commandlist
    hRes = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_directCommandListAllocator.Get(), nullptr,
        IID_PPV_ARGS(&m_commandList));
    ILL_ASSERT(ILL_TEXT("CommandList creation failed"), SUCCEEDED(hRes));

    m_commandList->Close();
}

void Illulu::Renderer::_CreateSwapChain(HWND hWnd, i32 width, i32 height) noexcept
{
    HRESULT hRes;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc
    {
        .Width = static_cast<u32>(width),
        .Height = static_cast<u32>(height),
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .Stereo = false,
        .SampleDesc
        {
            .Count   = 1,
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

    hRes = m_DXGIFactory->CreateSwapChainForHwnd(m_commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, swapChainV1.GetAddressOf());
    ILL_ASSERT(ILL_TEXT("Swapchain creation failed"), SUCCEEDED(hRes));

    hRes = swapChainV1.As<IDXGISwapChainIll>(&m_DXGISwapChain);
    ILL_ASSERT(ILL_TEXT("Swapchain conversion failed"), SUCCEEDED(hRes));
}

void Illulu::Renderer::_CreateRTVAndDSVDescriptorHeaps() noexcept
{
    m_RTVHeap.Init(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SWAPCHAIN_BUFFER_COUNT);
    m_DSVHeap.Init(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Illulu::Renderer::GetCurrentBackBufferView() noexcept
{
    return m_RTVHeap.GetCpuHandle(m_currBackBuffer);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Illulu::Renderer::GetDepthStencilView() noexcept
{
    return m_RTVHeap.GetCpuHandle(0);
}

void Illulu::Renderer::debug_LogAdaptersAndOutputs() noexcept
{
    HRESULT hRes;

    u32 i = 0;

    ComPtr<IDXGIAdapter1> adapterV1;

    while ((hRes = m_DXGIFactory->EnumAdapters1(i, adapterV1.ReleaseAndGetAddressOf())) != DXGI_ERROR_NOT_FOUND)
    {
        ComPtr<IDXGIAdapterIll> adapter;

        hRes = adapterV1.As<IDXGIAdapterIll>(&adapter);
        ILL_ASSERT(ILL_TEXT("Conversion from DXGIAdapter1 to DXGIAdapter4 didnt pass"), SUCCEEDED(hRes));

        DXGI_ADAPTER_DESC3 adapterDesc;
        adapter->GetDesc3(&adapterDesc);

        // here it won't compile if UNICODE not used, adapterDesc.Description is hardcoded wchar
        string adapterString = std::format(ILL_TEXT("Adapter: {}\n"), adapterDesc.Description);
        OutputDebugString(adapterString.c_str());

        u32 j = 0;
        ComPtr<IDXGIOutput> outputV0;
        while ((hRes = adapter->EnumOutputs(j, outputV0.ReleaseAndGetAddressOf())) == S_OK)
        {
            ComPtr<IDXGIOutputIll> output;
            hRes = outputV0.As<IDXGIOutputIll>(&output);
            ILL_ASSERT(ILL_TEXT("Conversion from DXGIOutput to DXGIOutput6 didnt pass"), SUCCEEDED(hRes));

            DXGI_OUTPUT_DESC1 outputDesc;
            hRes = output->GetDesc1(&outputDesc);
            ILL_ASSERT(ILL_TEXT("GetDesc1 was not successful"), SUCCEEDED(hRes));

            // here it won't compile if UNICODE not used, outputDesc.DeviceName is hardcoded wchar
            string outputString = std::format(ILL_TEXT("\tMonitor: {} \n"), outputDesc.DeviceName);
            OutputDebugString(outputString.c_str());

            std::vector<DXGI_MODE_DESC1> modeLists = _GetDisplayModes(output);

            for (const auto& x : modeLists)
            {
                f32 nom = static_cast<f32>(x.RefreshRate.Numerator);
                f32 denom = static_cast<f32>(x.RefreshRate.Denominator);
                f32 refreshRate = nom / denom;

                u32 width = x.Width;
                u32 height = x.Height;

                OutputDebugString(std::format(ILL_TEXT("\t\t{}x{}@{}Hz\n"), width, height, refreshRate).c_str());
            }
            j++;
        }
        ILL_ASSERT(ILL_TEXT("EnumAdapters failed: ppAdapter parameter might be NULL"), hRes == DXGI_ERROR_NOT_FOUND);

        i++;
    }

    ILL_ASSERT(ILL_TEXT("EnumAdapters failed: ppAdapter parameter might be NULL"), hRes == DXGI_ERROR_NOT_FOUND);

}

void Illulu::Renderer::debug_EnableDebugLayer() noexcept
{
    HRESULT hRes;
    
    ComPtr<ID3D12DebugIll> debugController;
    hRes = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
    ILL_ASSERT(ILL_TEXT("D3D12GetDebugInterface was not successful"), SUCCEEDED(hRes));
    
    debugController->EnableDebugLayer();
    debugController->SetEnableGPUBasedValidation(true);
}

void Illulu::DescriptorHeap::Init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE descHeapType, u32 capacity)
{
    ILL_ASSERT(ILL_TEXT("Initializing non-null heap"), !m_heap);
    ILL_ASSERT(ILL_TEXT("Initializing heap with invalid ID3D12Device*"), device);

    HRESULT hRes;

    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc
    {
        .Type = descHeapType,
        .NumDescriptors = capacity,
        .Flags = (descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) ?
                 D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        .NodeMask = 0
    };

    hRes = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(m_heap.GetAddressOf()));
    ILL_ASSERT(ILL_TEXT("CreateDescriptorHeap failed"), SUCCEEDED(hRes));
}

ID3D12DescriptorHeap* Illulu::DescriptorHeap::GetHeap() const
{
    return m_heap.Get();
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Illulu::DescriptorHeap::GetCpuHandle(u32 index)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = 
        CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heap->GetCPUDescriptorHandleForHeapStart());
    hCPU.Offset(index);
    return hCPU;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Illulu::DescriptorHeap::GetGpuHandle(u32 index)
{
    CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU =
        CD3DX12_GPU_DESCRIPTOR_HANDLE(m_heap->GetGPUDescriptorHandleForHeapStart());
    hGPU.Offset(index);
    return hGPU;
}
