#include "Renderer.h"

#include "Common.h"
#include "WindowsMin.h"

#include "RHI/Util.h"

#include "Vector.h"
#include "Core/Filesystem.h"

namespace Illulu
{
    void Renderer::OnInitialize(HWND hWnd)
    {
        // this should be already initialized correctly
        ILL_ASSERT(m_renderTargetHeight && m_renderTargetWidth);

        /* Initialize scissor rect and viewport */
        {
            m_viewport =
            {
                .TopLeftX = 0.0f,
                .TopLeftY = 0.0f,
                .Width = static_cast<f32>(m_renderTargetWidth),
                .Height = static_cast<f32>(m_renderTargetHeight),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f
            };

            m_scissorRect =
            {
                .left = 0,
                .top = 0,
                .right = m_renderTargetWidth,
                .bottom = m_renderTargetHeight,
            };
        }

        u32 factoryFlags = 0;

    #if defined(_DEBUG)
        factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
        EnableDebugLayer();
    #endif

        // create DXGIFactory
        WIN_CHECK(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory)));
        // create device through adapters
        ComPtr<IDXGIAdapter1> adapterV1;

        // method to find device which supports feature_level
        bool found = false;
        for (i32 i = 0; WIN_OK(m_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapterV1))); i++)
        {
            HRESULT hRes = D3D12CreateDevice(adapterV1.Get(), FEATURE_LEVEL, IID_PPV_ARGS(&m_device));

            if (SUCCEEDED(hRes))
            {
                found = true;
                break;
            }
        }
        ILL_ASSERT(found);

        m_infoQueue.CreateAndSetCallback(m_device.Get());

        /* Create command queue */
        {
            D3D12_COMMAND_QUEUE_DESC queueDesc
            {
                .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
                .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
                .NodeMask = 0
            };

            WIN_CHECK(m_device->CreateCommandQueue(
                &queueDesc,
                IID_PPV_ARGS(&m_commandQueue)
            ));
        }

        /* Create swapchain  */
        {
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc
            {
                .Width = static_cast<u32>(m_renderTargetWidth),
                .Height = static_cast<u32>(m_renderTargetHeight),
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .Stereo = false,
                .SampleDesc
                {
                    .Count = 1,
                    .Quality = 0
                },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = FRAMEBUFFER_COUNT,
                .Scaling = DXGI_SCALING_NONE,
                .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
                .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
                .Flags = 0
            };

            ComPtr<IDXGISwapChain1> swapChainV1;

            WIN_CHECK(m_factory->CreateSwapChainForHwnd(
                m_commandQueue.Get(),
                hWnd,
                &swapChainDesc,
                nullptr,
                nullptr,
                swapChainV1.GetAddressOf()
            ));

            WIN_CHECK(swapChainV1.As<IDXGISwapChainIll>(&m_swapchain));
        }

        m_frameIndex = static_cast<u8>(m_swapchain->GetCurrentBackBufferIndex());

        /* Disable fullscreen */
        if (DISABLE_FULLSCREEN)
        {
            WIN_CHECK(m_factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
        }

        /* Create RTV's descriptor heap */
        {
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc
            {
                .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                .NumDescriptors = FRAMEBUFFER_COUNT,
                .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                .NodeMask = 0
            };
            WIN_CHECK(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

            m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }

        /* Create frame resources (RTVs and command allocators) */
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

            // for each frame create rtv and command allocator
            for (u32 i = 0; i < FRAMEBUFFER_COUNT; i++)
            {
                WIN_CHECK(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
                m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);

                rtvHandle.Offset(1, m_rtvDescriptorSize);

                WIN_CHECK(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandListAllocators[i])))
            }
        }

        /* Create empty root signature */
        {
            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
            rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

            ComPtr<ID3DBlob> signature{};
            ComPtr<ID3DBlob> error{};

            WIN_CHECK(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf()));
            
            WIN_CHECK(m_device->CreateRootSignature(
                0, 
                signature->GetBufferPointer(), 
                signature->GetBufferSize(), 
                IID_PPV_ARGS(&m_rootSignature)
            ));
        }

        /* Create pipeline state (includes compiling and loading shaders) */
        {
            Vector<byte> vertexShaderData = Filesystem::ReadBinaryBlobFromFile(String(L"Basic_VSMain.cso"));
            Vector<byte> pixelShaderData = Filesystem::ReadBinaryBlobFromFile(String(L"Basic_PSMain.cso"));

            ILL_ASSERT(!vertexShaderData.empty() && !pixelShaderData.empty());

            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc
            {
                .pRootSignature{m_rootSignature.Get()},
                .VS{CD3DX12_SHADER_BYTECODE(vertexShaderData.data(), vertexShaderData.size())},
                .PS{CD3DX12_SHADER_BYTECODE(pixelShaderData.data(), pixelShaderData.size())},
                .DS{},
                .HS{},
                .GS{},
                .StreamOutput{},
                .BlendState{CD3DX12_BLEND_DESC(D3D12_DEFAULT)},
                .SampleMask{U32_MAX},
                .RasterizerState{CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT)},
                .DepthStencilState
                {
                    .DepthEnable{false},
                    .DepthWriteMask{},
                    .DepthFunc{},
                    .StencilEnable{false},
                    .StencilReadMask{},
                    .StencilWriteMask{},
                    .FrontFace{},
                    .BackFace{}
                },
                .InputLayout{Vertex1::GetInputLayoutDesc()},
                .IBStripCutValue{},
                .PrimitiveTopologyType{D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE},
                .NumRenderTargets{1},
                .RTVFormats{DXGI_FORMAT_R8G8B8A8_UNORM},
                .DSVFormat{},
                .SampleDesc
                {
                    .Count{1},
                    .Quality{}
                },
                .NodeMask{},
                .CachedPSO{},
                .Flags{}
            };

            WIN_CHECK(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
        }

        /* Create command list */
        {
            WIN_CHECK(m_device->CreateCommandList(
                0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_commandListAllocators[m_frameIndex].Get(),
                m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

            // command list is created in recording state -> close it
            WIN_CHECK(m_commandList->Close());
        }

        /* Create vertex buffer */
        {
            std::array<Vertex1, 3> triangleVertices
            {
                {
                    { { 0.0f, 0.25f * _GetRenderTargetAspectRatio(), 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f } },
                    { { 0.25f, -0.25f * _GetRenderTargetAspectRatio(), 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f } },
                    { { -0.25f, -0.25f * _GetRenderTargetAspectRatio(), 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f } }
                }
            };

            constexpr u64 vertexBufferSize = sizeof(triangleVertices);

            // create the vertex buffer and initialize it with triangle's values
            {
                CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
                CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

                WIN_CHECK(m_device->CreateCommittedResource(
                    &heapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&m_vertexBuffer)));

                u8* pVertexDataBegin{nullptr};
                CD3DX12_RANGE readRange(0, 0);
                WIN_CHECK(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
                memcpy(pVertexDataBegin, triangleVertices.data(), vertexBufferSize);
                m_vertexBuffer->Unmap(0, nullptr);

                m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
                m_vertexBufferView.StrideInBytes = sizeof(Vertex1);
                m_vertexBufferView.SizeInBytes = vertexBufferSize;
            }
        }

        /* Create synchronization objects */
        {
            WIN_CHECK(m_device->CreateFence(
                m_fenceValues[m_frameIndex], 
                D3D12_FENCE_FLAG_NONE, 
                IID_PPV_ARGS(&m_fence)
            ));

            m_fenceValues[m_frameIndex]++;
            

            // create event handle
            m_fenceEvent.Attach(CreateEvent(nullptr, false, false, nullptr));
            ILL_ASSERT(m_fenceEvent.IsValid());

            _WaitForGpu();
        }

        m_initialized = true;
    }

    void Renderer::OnUpdate()
    {
    }

    void Renderer::OnRender()
    {
        _FeedCommandList();

        // Execute the command list.
        ID3D12CommandList* ppCommandLists[] = {m_commandList.Get()};
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


        // Present the frame.
        WIN_CHECK(m_swapchain->Present(1, 0));

        _MoveToNextFrame();
    }

    void Renderer::OnShutdown()
    {
        _WaitForGpu();

        m_infoQueue.Destroy();

        if (m_fenceEvent.IsValid())
        {
            m_fenceEvent.Close();
        }
    }

    void Renderer::UpdateRenderTargetSize(i32 newWidth, i32 newHeight)
    {
        m_renderTargetWidth = newWidth;
        m_renderTargetHeight = newHeight;
        
        if (!m_initialized)
            return;

        _WaitForGpu();

        m_viewport =
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<f32>(m_renderTargetWidth),
            .Height = static_cast<f32>(m_renderTargetHeight),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };

        m_scissorRect =
        {
            .left = 0,
            .top = 0,
            .right = m_renderTargetWidth,
            .bottom = m_renderTargetHeight
        };

        const u64 fenceValue = m_fenceValues[m_frameIndex];

        /* Release current RTVs */
        for (u32 i = 0; i < FRAMEBUFFER_COUNT; i++)
        { 
            m_renderTargets[i].Reset();
            m_fenceValues[i] = fenceValue;
        }

        DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
        WIN_CHECK(m_swapchain->GetDesc1(&swapchainDesc));

        WIN_CHECK(m_swapchain->ResizeBuffers(
            FRAMEBUFFER_COUNT,
            m_renderTargetWidth,
            m_renderTargetHeight,
            swapchainDesc.Format,
            swapchainDesc.Flags
        ));

        m_frameIndex = static_cast<u8>(m_swapchain->GetCurrentBackBufferIndex());

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (u32 i = 0; i < FRAMEBUFFER_COUNT; i++)
        {
            WIN_CHECK(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
            m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);

            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    void Renderer::_FeedCommandList()
    {
        // reset command list allocator -> this can only happen when command list associated with the allocator has finished
        WIN_CHECK(m_commandListAllocators[m_frameIndex]->Reset());

        // command list can be reset immediately after calling ExecuteCommandLists on it
        WIN_CHECK(m_commandList->Reset(m_commandListAllocators[m_frameIndex].Get(), m_pipelineState.Get()));

        // setting states
        m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);

        // set backbuffer to be rendered at
        D3D12_RESOURCE_BARRIER resourceBarrier
        {
            CD3DX12_RESOURCE_BARRIER::Transition(
                m_renderTargets[m_frameIndex].Get(),
                D3D12_RESOURCE_STATE_PRESENT, 
                D3D12_RESOURCE_STATE_RENDER_TARGET)
        };

        m_commandList->ResourceBarrier(1, &resourceBarrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
            m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), 
            m_frameIndex, 
            m_rtvDescriptorSize
        );
        m_commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

        // record commands
        constexpr Array<float, 4> clearColor{0.0f, 0.2f, 0.4f, 1.0f};
        m_commandList->ClearRenderTargetView(rtvHandle, clearColor.data(), 0, nullptr);
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        m_commandList->DrawInstanced(3, 1, 0, 0);

        // present backbuffer
        resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                m_renderTargets[m_frameIndex].Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PRESENT
        );
        
        m_commandList->ResourceBarrier(1, &resourceBarrier);
        WIN_CHECK(m_commandList->Close());
    }

    void Renderer::_WaitForGpu()
    {
        // Schedule a Signal command in the queue.
        WIN_CHECK(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

        // Wait until the fence has been processed.
        WIN_CHECK(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent.Get()));
        WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, false);

        // Increment the fence value for the current frame.
        m_fenceValues[m_frameIndex]++;
    }

    void Renderer::_MoveToNextFrame()
    {
        // Schedule a Signal command in the queue.
        const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
        WIN_CHECK(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

        // Update the frame index.
        m_frameIndex = static_cast<u8>(m_swapchain->GetCurrentBackBufferIndex());

        // If the next frame is not ready to be rendered yet, wait until it is ready.
        if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
        {
            WIN_CHECK(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent.Get()));
            WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, false);
        }

        // Set the fence value for the next frame.
        m_fenceValues[m_frameIndex] = currentFenceValue + 1;
    }

    f32 Renderer::_GetRenderTargetAspectRatio() const
    {
        ILL_ASSERT(m_renderTargetHeight > 0);
        return static_cast<f32>(m_renderTargetWidth) / static_cast<f32>(m_renderTargetHeight);
    }

    ID3D12Resource* Renderer::_GetCurrentBackbuffer() noexcept
    {
        return m_renderTargets[m_frameIndex].Get();
    }
}