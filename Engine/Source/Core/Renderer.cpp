#include "Renderer.h"

#include "Common.h"
#include "WindowsMin.h"

namespace Illulu
{
    void Renderer::OnInitialize(HWND hWnd, i32 clientWidth, i32 clientHeight)
    {
        u32 factoryFlags = 0;

    #if defined(_DEBUG)
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

        m_infoQueue.CreateAndSetCallback(m_device.Get());

        // create fence throught newly created device
        WIN_CHECK(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

        m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
        ILL_ASSERT(m_fenceEvent);

        // create command queue, allocator, list
        _CreateCommandObjects();

        // reopen command list for initialization commands
        WIN_CHECK(m_directCommandListAllocator->Reset());
        WIN_CHECK(m_commandList->Reset(m_directCommandListAllocator.Get(), nullptr));

        // create swapchain
        _CreateSwapChain(hWnd, clientWidth, clientHeight);

        // create RTV and DSV descriptor heaps
        _CreateRTVAndDSVDescriptorHeaps();
        
        // create descriptor heap for CBVs, SRVs, UAVs
        m_cbvSrvUavHeap.Initialize(m_device.Get(), CBV_SRV_UAV_HEAP_CAPACITY);

        // init imgui
        _ImGuiInit(hWnd);

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

        m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, _GetDepthStencilView());

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

        WIN_CHECK(m_commandList->Close());
        ID3D12CommandList* commandLists[] = {m_commandList.Get()};
        m_commandQueue->ExecuteCommandLists(1, commandLists);
        _FlushCommandQueue();

        m_currBackBuffer = static_cast<u8>(m_DXGISwapChain->GetCurrentBackBufferIndex());
    }

    void Renderer::OnUpdate()
    {
        _ImGuiBeginFrame();
        
        WIN_CHECK(m_directCommandListAllocator->Reset());
        WIN_CHECK(m_commandList->Reset(m_directCommandListAllocator.Get(), nullptr));

        const CD3DX12_RESOURCE_BARRIER presentToRenderTarget =
            CD3DX12_RESOURCE_BARRIER::Transition(
                _GetCurrentBackbuffer(),
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_RENDER_TARGET
            );
        m_commandList->ResourceBarrier(1, &presentToRenderTarget);

        auto rtv = _GetCurrentBackbufferView();

        m_commandList->OMSetRenderTargets(1, &rtv, false, nullptr);

        const f32 clearColor[] = {0.08f, 0.10f, 0.15f, 1.0f};

        m_commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);


        _ImGuiEndFrame();

        const CD3DX12_RESOURCE_BARRIER renderTargetToPresent =
            CD3DX12_RESOURCE_BARRIER::Transition(
                _GetCurrentBackbuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PRESENT
            );
        m_commandList->ResourceBarrier(1, &renderTargetToPresent);


        WIN_CHECK(m_commandList->Close());

        ID3D12CommandList* commandLists[] = {m_commandList.Get()};
        m_commandQueue->ExecuteCommandLists(1, commandLists);

        WIN_CHECK(m_DXGISwapChain->Present(1, 0));

        m_currBackBuffer = static_cast<u8>(m_DXGISwapChain->GetCurrentBackBufferIndex());

        _FlushCommandQueue();

    }

    void Renderer::OnShutdown()
    {
        _FlushCommandQueue();

        _ImGuiDestroy();
        
        m_infoQueue.Destroy();
    }

    void Renderer::ResizeBackbuffer()
    {
    }

    void Renderer::_FlushCommandQueue()
    {
        m_currentFence++;

        WIN_CHECK(m_commandQueue->Signal(m_fence.Get(), m_currentFence));

        if (m_fence->GetCompletedValue() < m_currentFence)
        {
            WIN_CHECK(m_fence->SetEventOnCompletion(m_currentFence, m_fenceEvent));
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
    }

    ID3D12Resource* Renderer::_GetCurrentBackbuffer() noexcept
    {
        return m_swapChainBuffer[m_currBackBuffer].Get();
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
            .Flags = 0
        };

        ComPtr<IDXGISwapChain1> swapChainV1;

        WIN_CHECK(m_DXGIFactory->CreateSwapChainForHwnd(m_commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, swapChainV1.GetAddressOf()));

        WIN_CHECK(swapChainV1.As<IDXGISwapChainIll>(&m_DXGISwapChain));
    }

    void Renderer::_CreateRTVAndDSVDescriptorHeaps() noexcept
    {
        ILL_ASSERT(!m_RTVHeap.IsInitialized() && !m_DSVHeap.IsInitialized());
        
        m_RTVHeap.Initialize(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SWAPCHAIN_BUFFER_COUNT);
        m_DSVHeap.Initialize(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

        ILL_ASSERT(m_RTVHeap.IsInitialized() && m_DSVHeap.IsInitialized());
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE Renderer::_GetCurrentBackbufferView() noexcept
    {
        return m_RTVHeap.GetCpuHandle(m_currBackBuffer);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE Renderer::_GetDepthStencilView() noexcept
    {
        return m_DSVHeap.GetCpuHandle(0);
    }

    void Renderer::_ImGuiInit(HWND hWnd)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui_ImplWin32_Init(hWnd);

        ImGui_ImplDX12_InitInfo initInfo = {};
        initInfo.Device = m_device.Get();
        initInfo.CommandQueue = m_commandQueue.Get();
        initInfo.NumFramesInFlight = SWAPCHAIN_BUFFER_COUNT;
        initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

        initInfo.SrvDescriptorHeap = m_cbvSrvUavHeap.GetHeap();
        initInfo.SrvDescriptorAllocFn =
            [](ImGui_ImplDX12_InitInfo* info,
               D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle,
               D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle)
            {
                auto* renderer = static_cast<Renderer*>(info->UserData);

                renderer->m_imguiFontSrvIndex =
                    renderer->m_cbvSrvUavHeap.GetNextFreeIndex();

                *outCpuHandle =
                    renderer->m_cbvSrvUavHeap.GetCpuHandle(renderer->m_imguiFontSrvIndex);

                *outGpuHandle =
                    renderer->m_cbvSrvUavHeap.GetGpuHandle(renderer->m_imguiFontSrvIndex);
            };

        initInfo.SrvDescriptorFreeFn =
            [](ImGui_ImplDX12_InitInfo* info,
               D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
               D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
            {
                (void)cpuHandle;
                (void)gpuHandle;

                auto* renderer = static_cast<Renderer*>(info->UserData);

                if (renderer->m_imguiFontSrvIndex != UINT32_MAX)
                {
                    renderer->m_cbvSrvUavHeap.ReleaseIndex(renderer->m_imguiFontSrvIndex);
                    renderer->m_imguiFontSrvIndex = UINT32_MAX;
                }
            };

        initInfo.UserData = this;

        WIN_CHECK(ImGui_ImplDX12_Init(&initInfo));
    }

    void Renderer::_ImGuiBeginFrame()
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
    }

    void Renderer::_ImGuiEndFrame()
    {
        ImGui::Render();

        ID3D12DescriptorHeap* heaps[] =
        {
            m_cbvSrvUavHeap.GetHeap()
        };

        m_commandList->SetDescriptorHeaps(1, heaps);

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());
    }

    void Renderer::_ImGuiDestroy()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }



    void Renderer::debug_EnableDebugLayer()
    {
        ComPtr<ID3D12DebugIll> debugController;
        WIN_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));

        debugController->EnableDebugLayer();
        debugController->SetEnableGPUBasedValidation(true);
    }
}
