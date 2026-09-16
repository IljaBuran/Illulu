#include "Renderer.hpp"

#include "Common.hpp"
#include "WindowsMin.hpp"

#include "Vector.hpp"
#include "String.hpp"

#include "Core/Filesystem.hpp"

#include "D3D12/Util.hpp"

#include "Data.hpp"

#include "Scene/Model.hpp"

namespace Illulu
{
    void Renderer::OnInitialize(HWND hWnd)
    {
        INFO(L"*** [DX12] Initialization start ***");
        #pragma comment(lib, "imgui.lib")

        // this should be already initialized correctly
        ILL_ASSERT(m_renderTargetHeight && m_renderTargetWidth);

        /* Initialize scissor rect and viewport */
        {
            m_viewport =
            {
                .TopLeftX{ 0.0f },
                .TopLeftY{ 0.0f },
                .Width{ static_cast<f32>(m_renderTargetWidth) },
                .Height{ static_cast<f32>(m_renderTargetHeight) },
                .MinDepth{ 0.0f },
                .MaxDepth{ 1.0f }
            };

            m_scissorRect =
            {
                .left{ 0 },
                .top{ 0 },
                .right{ static_cast<i32>(m_renderTargetWidth) },
                .bottom{ static_cast<i32>(m_renderTargetHeight) },
            };
        }

        D3D12::Factory factory{};
        m_device.Initialize(factory);

        m_commandQueue.Create(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);

        /* Create CBV's descriptor heap */
        m_cbvSrvUavHeap.Initialize(m_device, CBV_SRV_UAV_HEAP_CAPACITY);

        /* Create DSV's descriptor heap */
        m_dsvHeap.Initialize(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, FRAMEBUFFER_COUNT);

        /* Initialize swap chain */
        m_swapChain.Create(factory, m_device, m_commandQueue, hWnd, m_renderTargetWidth, m_renderTargetHeight);

        _BuildFrameResources();

        /* Create a root signature consisting of a descriptor table with single CBV */
        {
            D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData
            {
                .HighestVersion{ D3D_ROOT_SIGNATURE_VERSION_1_1 }
            };

            Array<CD3DX12_ROOT_PARAMETER1, 2> rootParameters{};
            rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX); // b0: pass
            rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX); // b1: object

            D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags
            {
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
            };

            CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
            rootSignatureDesc.Init_1_1(static_cast<u32>(rootParameters.size()), rootParameters.data(), 0, nullptr, rootSignatureFlags);

            ComPtr<ID3DBlob> signature{};
            ComPtr<ID3DBlob> error{};

            // Serialize -> it is possible to save to a file and then load it from there...
            WIN_CHECK(D3DX12SerializeVersionedRootSignature(
                &rootSignatureDesc, 
                featureData.HighestVersion, 
                &signature, 
                &error
            ));

            WIN_CHECK(m_device->CreateRootSignature(
                0,
                signature->GetBufferPointer(),
                signature->GetBufferSize(),
                IID_PPV_ARGS(&m_rootSignature)
            ));
        }

        /* Create pipeline state (includes compiling and loading shaders) */
        {
            m_shader.Init(
                L"Shaders/Basic.hlsl",
                D3D12::ShaderType::VERTEX | D3D12::ShaderType::PIXEL);

            m_shader.Compile();

            MulticastDelegate<> del;
            del.Add<Renderer, &Renderer::RecompileShader>(this);
            m_fileWatcher.Intialize(L"Shaders/", L"Basic.hlsl", std::move(del));

            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc
            {
                .pRootSignature{ m_rootSignature.Get() },
                .VS{ m_shader.GetBlob(D3D12::ShaderType::VERTEX) },
                .PS{ m_shader.GetBlob(D3D12::ShaderType::PIXEL) },
                .DS{},
                .HS{},
                .GS{},
                .StreamOutput{},
                .BlendState{ CD3DX12_BLEND_DESC(D3D12_DEFAULT) },
                .SampleMask{ U32_MAX },
                .RasterizerState{ CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT) },
                .DepthStencilState
                {
                    .DepthEnable{ false },
                    .DepthWriteMask{},
                    .DepthFunc{},
                    .StencilEnable{ false },
                    .StencilReadMask{},
                    .StencilWriteMask{},
                    .FrontFace{},
                    .BackFace{}
                },
                .InputLayout{ ColorVertex::GetInputLayoutDesc() },
                .IBStripCutValue{},
                .PrimitiveTopologyType{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE },
                .NumRenderTargets{ 1 },
                .RTVFormats{ DXGI_FORMAT_R8G8B8A8_UNORM },
                .DSVFormat{},
                .SampleDesc
                {
                    .Count{ 1 },
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
            m_commandList.Create(
                m_device,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_pCurrFrameRes->m_commandAllocator,
                m_pipelineState.Get()
            );

            // command list is created in recording state -> close it
            WIN_CHECK(m_commandList->Close());
        }

        /* Create synchronization objects */
        {
            WIN_CHECK(m_device->CreateFence(
                0,
                D3D12_FENCE_FLAG_NONE,
                IID_PPV_ARGS(&m_fence)
            ));

            //m_fenceValues[m_swapChain.m_backbufferIndex]++;

            // create event handle
            m_fenceEvent.Attach(
                CreateEvent(nullptr, false, false, nullptr)
            );

            ILL_ASSERT(m_fenceEvent.IsValid());
        }


        u32 totalSizeNeeded{ sizeof(boxVertices) + sizeof(boxIndices) };
        // Create default heap resource, we'll copy the vertices and indices here from upload buffer
        {
            auto heapProps{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
            auto resDesc{ CD3DX12_RESOURCE_DESC::Buffer(totalSizeNeeded) };

            WIN_CHECK(m_device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &resDesc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&m_vertexIndexBufferGPU)
            ));
        }

        // Create upload heap resource and copy data into it
        {
            // we'll use the same config, but different heap type
            auto heapProps{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
            auto resDesc{ CD3DX12_RESOURCE_DESC::Buffer(totalSizeNeeded) };

            WIN_CHECK(m_device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &resDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_uploadBuffer)
            ));

            auto mapRange{ CD3DX12_RANGE(0, 0) };
            m_uploadBuffer->Map(0, &mapRange, reinterpret_cast<void**>(&m_mappedUploadBuffer));

            // copy vertices and indices into upload buffer
            std::memcpy(m_mappedUploadBuffer, boxVertices.data(), sizeof(boxVertices));
            std::memcpy(m_mappedUploadBuffer + sizeof(boxVertices), boxIndices.data(), sizeof(boxIndices));

            m_uploadBuffer->Unmap(0, nullptr);
        }

        // VERY IMPORTANT HERE: 
        // we have closed the commandlist after it's creation, need to reopen now
        WIN_CHECK(m_commandList->Reset(m_pCurrFrameRes->m_commandAllocator, nullptr));

        // transition default buffer to COPY_DEST, copy and then transition back
        {
            D3D12_RESOURCE_BARRIER barrier
            {
                CD3DX12_RESOURCE_BARRIER::Transition(
                    m_vertexIndexBufferGPU.Get(),
                    D3D12_RESOURCE_STATE_COMMON,
                    D3D12_RESOURCE_STATE_COPY_DEST)
            };

            m_commandList->ResourceBarrier(1, &barrier);

            m_commandList->CopyBufferRegion(
                m_vertexIndexBufferGPU.Get(), 0,
                m_uploadBuffer.Get(), 0,
                totalSizeNeeded
            );

            barrier = 
            {
                CD3DX12_RESOURCE_BARRIER::Transition(
                    m_vertexIndexBufferGPU.Get(),
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    D3D12_RESOURCE_STATE_GENERIC_READ)
            };

            m_commandList->ResourceBarrier(1, &barrier);
        }

        // execute the copy from upload to default
        {
            WIN_CHECK(m_commandList->Close());

            Array<ID3D12CommandList*, 1> lists{ m_commandList.GetCommandListPtr() };
            m_commandQueue->ExecuteCommandLists(static_cast<u32>(lists.size()), lists.data());

            _FlushCommandQueue();
        }

        // create vertex buffer view
        {
            m_vertexBufferView =
            {
                .BufferLocation{m_vertexIndexBufferGPU->GetGPUVirtualAddress()},
                .SizeInBytes{sizeof(boxVertices)},
                .StrideInBytes{sizeof(ColorVertex)}
            };
        }

        // create index buffer view
        {
            m_indexBufferView =
            {
                .BufferLocation{m_vertexIndexBufferGPU->GetGPUVirtualAddress() + sizeof(boxVertices)},
                .SizeInBytes{sizeof(boxIndices)},
                .Format{DXGI_FORMAT_R16_UINT}
            };
        }

        // TODO: use UploadBuffer instead

        m_linearAllocator = std::make_unique<GraphicsMemory>(m_device);

        ///* create constant buffers */
        //{
        //    // PER OBJECT
        //    constexpr u32 objCbSize{ (u32)AlignUp(sizeof(ObjectConstants), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT) };

        //    auto heapProps{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
        //    auto resDesc{ CD3DX12_RESOURCE_DESC::Buffer(objCbSize) };

        //    WIN_CHECK(m_device->CreateCommittedResource(
        //        &heapProps,
        //        D3D12_HEAP_FLAG_NONE,
        //        &resDesc,
        //        D3D12_RESOURCE_STATE_GENERIC_READ,
        //        nullptr,
        //        IID_PPV_ARGS(&m_perObjectUploadBuffer)
        //    ));

        //    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc
        //    {
        //        .BufferLocation{m_perObjectUploadBuffer->GetGPUVirtualAddress()},
        //        .SizeInBytes{objCbSize}
        //    };

        //    m_cbPerObjectIndex = m_cbvSrvUavHeap.GetNextFreeIndex();
        //    m_device->CreateConstantBufferView(&cbvDesc, m_cbvSrvUavHeap.GetCpuHandle(m_cbPerObjectIndex));

        //    auto mapRange{ CD3DX12_RANGE(0, 0) };
        //    m_perObjectUploadBuffer->Map(0, &mapRange, reinterpret_cast<void**>(&m_pPerObjectMapped));

        //    // PER PASS
        //    constexpr u32 passCbSize{ 
        //        static_cast<u32>(AlignUp(sizeof(PassConstants), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
        //    };
        //    resDesc.Width = passCbSize;

        //    WIN_CHECK(m_device->CreateCommittedResource(
        //        &heapProps,
        //        D3D12_HEAP_FLAG_NONE,
        //        &resDesc,
        //        D3D12_RESOURCE_STATE_GENERIC_READ,
        //        nullptr,
        //        IID_PPV_ARGS(&m_perPassUploadBuffer)
        //    ));

        //    cbvDesc =
        //    {
        //        .BufferLocation{m_perPassUploadBuffer->GetGPUVirtualAddress()},
        //        .SizeInBytes{passCbSize}
        //    };

        //    m_cbPerPassIndex = m_cbvSrvUavHeap.GetNextFreeIndex();
        //    m_device->CreateConstantBufferView(&cbvDesc, m_cbvSrvUavHeap.GetCpuHandle(m_cbPerPassIndex));
        //    m_perPassUploadBuffer->Map(0, &mapRange, reinterpret_cast<void**>(&m_pPerPassMapped));
        //}

        _ImGuiInit(hWnd);

        m_initialized = true;

        Vector<Mesh> model{ LoadModel("Assets/truck/scene.gltf") };

        INFO(L"*** [DX12] Initialization successful ***");
    }

    void Renderer::OnUpdate()
    {
        m_fileWatcher.OnUpdate();

        m_currFrameResIdx = (m_currFrameResIdx + 1) % numFrameResources;
        m_pCurrFrameRes = &m_frameResources[m_currFrameResIdx];

        // check if we need to wait
        if (m_pCurrFrameRes->m_fence != 0 && m_fence->GetCompletedValue() < m_pCurrFrameRes->m_fence)
        {
            WIN_CHECK(m_fence->SetEventOnCompletion(
                m_pCurrFrameRes->m_fence, 
                m_fenceEvent.Get()
            ));

            //DWORD res{ WaitForSingleObject(m_fenceEvent.Get(), INFINITE) };
            //ILL_ASSERT(res == WAIT_OBJECT_0);
            ILL_VERIFY(WaitForSingleObject(m_fenceEvent.Get(), INFINITE) == WAIT_OBJECT_0);
        }

        // --- per object: world matrix ---
        static f32 angleX{ 0.0f };
        static f32 angleY{ 0.0f };

        angleX -= XM_PI * m_deltaY / (static_cast<f32>(m_renderTargetHeight) / 2.0f);
        angleY -= XM_PI * m_deltaX / (static_cast<f32>(m_renderTargetWidth) / 2.0f);
        
        // restrict X rotation to (-pi, pi);
        angleX = Clamp(angleX, -XM_PI / 2, XM_PI / 2);

        m_deltaX = 0;
        m_deltaY = 0;

        // --- per object: model matrix ---
        const XMMATRIX world{ DirectX::XMMatrixRotationY(angleY) * DirectX::XMMatrixRotationX(angleX)};
        ObjectConstants objData{};
        XMStoreFloat4x4(&objData.M, DirectX::XMMatrixTranspose(world)); // HLSL expects column-major by default, transpose row-major XMMatrix
        m_objectAllocation = m_linearAllocator->AllocateConstant(objData);

        // --- per pass: view-projection matrix ---
        const XMVECTOR eye{ XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f) };
        const XMVECTOR target{ XMVectorZero() };
        const XMVECTOR up{ XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };
        const XMMATRIX view{ DirectX::XMMatrixLookAtLH(eye, target, up) };

        const f32 aspect{ m_swapChain.GetAspectRatio() };
        const XMMATRIX proj{ DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.1f, 100.0f) };

        PassConstants passData{}; 
        XMStoreFloat4x4(&passData.VP, DirectX::XMMatrixTranspose(view * proj)); // HLSL expects column-major by default, transpose row-major XMMatrix
        m_pCurrFrameRes->m_passCB.CopyData(0, passData);
    }

    void Renderer::OnRender()
    {
        _FeedCommandList();

        // Execute the command list.
        const Array<ID3D12CommandList*, 1> commandLists{ m_commandList };

        m_linearAllocator->Commit(m_commandQueue);

        m_commandQueue->ExecuteCommandLists(static_cast<u32>(commandLists.size()), commandLists.data());
        
        // Present the frame.
        WIN_CHECK(m_swapChain->Present(1, 0));

        _EndFrame();
    }

    void Renderer::OnShutdown()
    {
        _FlushCommandQueue();

        _ImGuiShutdown();

        if (m_fenceEvent.IsValid())
        {
            m_fenceEvent.Close();
        }
    }

    void Renderer::RecompileShader()
    {
        _FlushCommandQueue();

        // compile with permissive mode -> when fails, it won't throw
        if (!m_shader.Compile(true))
            return;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc
        {
            .pRootSignature{ m_rootSignature.Get() },
            .VS{m_shader.GetBlob(D3D12::ShaderType::VERTEX)},
            .PS{m_shader.GetBlob(D3D12::ShaderType::PIXEL)},
            .DS{},
            .HS{},
            .GS{},
            .StreamOutput{},
            .BlendState{CD3DX12_BLEND_DESC(D3D12_DEFAULT)},
            .SampleMask{ U32_MAX },
            .RasterizerState{ CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT) },
            .DepthStencilState
            {
                .DepthEnable{ false },
                .DepthWriteMask{},
                .DepthFunc{},
                .StencilEnable{ false },
                .StencilReadMask{},
                .StencilWriteMask{},
                .FrontFace{},
                .BackFace{}
            },
            .InputLayout{ ColorVertex::GetInputLayoutDesc() },
            .IBStripCutValue{},
            .PrimitiveTopologyType{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE },
            .NumRenderTargets{ 1 },
            .RTVFormats{ DXGI_FORMAT_R8G8B8A8_UNORM },
            .DSVFormat{},
            .SampleDesc
            {
                .Count{ 1 },
                .Quality{}
            },
            .NodeMask{},
            .CachedPSO{},
            .Flags{}
        };

        ComPtr<ID3D12PipelineState> newPso;
        HRESULT hRes{ m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&newPso)) };

        if (!WIN_OK(hRes))
        {
            FATAL(L"[SHADER] Recompilation didn't succeed: {}", TranslateHResult(hRes));
            return;
        }

        WIN_CHECK(m_device->CreateGraphicsPipelineState(
            &psoDesc,
            IID_PPV_ARGS(&newPso)
        ));

        m_pipelineState = newPso;

        m_commandList->Reset(m_pCurrFrameRes->m_commandAllocator, m_pipelineState.Get());
        m_commandList->SetPipelineState(m_pipelineState.Get());
        m_commandList->Close();

        INFO(L"Shader recompiled");
    }

    void Renderer::UpdateRenderTargetSize(i32 newWidth, i32 newHeight)
    {
        m_renderTargetWidth = newWidth;
        m_renderTargetHeight = newHeight;

        if (!m_initialized)
            return;

        _FlushCommandQueue();

        m_viewport =
        {
            .TopLeftX{ 0.0f },
            .TopLeftY{ 0.0f },
            .Width{ static_cast<f32>(m_renderTargetWidth) },
            .Height{ static_cast<f32>(m_renderTargetHeight) },
            .MinDepth{ 0.0f },
            .MaxDepth{ 1.0f }
        };

        m_scissorRect =
        {
            .left{0},
            .top{0},
            .right{ static_cast<i32>(m_renderTargetWidth) },
            .bottom{ static_cast<i32>(m_renderTargetHeight) }
        };

        m_swapChain.Resize(m_device, newWidth, newHeight);

        INFO(L"[DX12] Render target resized");
    }

    void Renderer::_ImGuiInit(HWND hWnd)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        //ImGuiIO& io = ImGui::GetIO();
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

        ILL_VERIFY(ImGui_ImplWin32_Init(hWnd));

        ImGui_ImplDX12_InitInfo initInfo{};
        initInfo.Device = m_device;
        initInfo.CommandQueue = m_commandQueue;
        initInfo.NumFramesInFlight = numFrameResources;
        initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
        initInfo.UserData = reinterpret_cast<void*>(&m_cbvSrvUavHeap);

        initInfo.SrvDescriptorHeap = m_cbvSrvUavHeap;
        initInfo.SrvDescriptorAllocFn =
            [](ImGui_ImplDX12_InitInfo* initInfo, D3D12_CPU_DESCRIPTOR_HANDLE* pCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* pGpuHandle)
        {
            CbvSrvUavHeap& heap{ *static_cast<CbvSrvUavHeap*>(initInfo->UserData) };

            const u32 index{ heap.GetNextFreeIndex() };

            *pCpuHandle = heap.GetCpuHandle(index);
            *pGpuHandle = heap.GetGpuHandle(index);
        };
        initInfo.SrvDescriptorFreeFn =
            [](ImGui_ImplDX12_InitInfo* initInfo, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, UNUSED D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
        {
            CbvSrvUavHeap& heap{ *static_cast<CbvSrvUavHeap*>(initInfo->UserData) };

            u32 index{ heap.GetIndex(cpuHandle) };
            heap.ReleaseIndex(index);
        };

        ILL_VERIFY(ImGui_ImplDX12_Init(&initInfo));

        ImGuiIO& io{ ImGui::GetIO() };

        io.Fonts->AddFontFromFileTTF(
            "C:/Windows/Fonts/consola.ttf",
            17.0f
        );
    }

    void Renderer::_ImGuiStartFrame()
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();

        ImGui::Begin("Console");
        {
            if (ImGui::BeginChild("ScrollRegion##"))
            {
                // Wrap items. TODO
                ImGui::PushTextWrapPos();
                {
                    auto& history{ Logger::GetLogHistory() };

                    // Display items.
                    for (u32 i{0}; i < history.Size(); i++)
                    {
                        const auto& log{ history[i] };
                
                        ImVec4 color{};

                        switch (log.type)
                        {
                            case LogType::INFO:
                            {
                                color = { 200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 1.0f };
                                break;
                            }
                            case LogType::WARNING:
                            {
                                color = { 242.0 / 255.0f, 140.0f / 255.0f, 40.0f / 255.0f, 1.0f };
                                break;
                            }
                            case LogType::FATAL:
                            {
                                color = { 210.0f / 255.0f, 4.0f / 255.0f, 45.0f / 255.0f, 1.0f };
                                break;
                            }
                            case LogType::COUNT:
                            {
                                WARNING(L"[MESSAGE_ERROR] Invalid LogType set");
                                continue;
                            }
                        }
                        ImGui::PushStyleColor(ImGuiCol_Text, color);
                        {
                            NarrowString narrow{ WideToUtf8(log.str) };
                            ImGui::TextUnformatted(narrow.data());
                        } 
                        ImGui::PopStyleColor();
                    }
                }
                ImGui::PopTextWrapPos();
            } 
            ImGui::EndChild();
        } 
        ImGui::End();
    }

    void Renderer::_ImGuiDraw()
    {
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList);
    }

    void Renderer::_ImGuiShutdown()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void Renderer::_BuildFrameResources()
    {
        for (auto& frameRes : m_frameResources)
        {
            frameRes.Create(m_device, 1);
        }

        m_currFrameResIdx = 0;
        m_pCurrFrameRes = &m_frameResources[m_currFrameResIdx];
    }

    void Renderer::_FeedCommandList()
    {
        // reset command list allocator -> this can only happen when command list associated with the allocator has finished
        //WIN_CHECK(m_commandListAllocators[m_swapChain.m_backbufferIndex]->Reset());
        WIN_CHECK(m_pCurrFrameRes->m_commandAllocator->Reset());

        // command list can be reset immediately after calling ExecuteCommandLists on it
        WIN_CHECK(m_commandList->Reset(m_pCurrFrameRes->m_commandAllocator, m_pipelineState.Get()));

        _ImGuiStartFrame();

        // setting states
        m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

        Array<ID3D12DescriptorHeap*, 1> ppHeaps{ m_cbvSrvUavHeap };
        m_commandList->SetDescriptorHeaps(static_cast<u32>(ppHeaps.size()), ppHeaps.data());

        m_commandList->SetGraphicsRootConstantBufferView(
            0, 
            m_objectAllocation.GpuAddress()
        );
        m_commandList->SetGraphicsRootConstantBufferView(
            1, 
            m_pCurrFrameRes->m_passCB.GetResourcePtr()->GetGPUVirtualAddress()
        );

        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);

        // set backbuffer to be rendered at
        D3D12_RESOURCE_BARRIER resourceBarrier
        {
            CD3DX12_RESOURCE_BARRIER::Transition(
                m_swapChain.m_buffers[m_swapChain.m_backbufferIndex].Get(),
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_RENDER_TARGET)
        };

        m_commandList->ResourceBarrier(1, &resourceBarrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
            m_swapChain.m_rtvHeap.GetCpuHandle(0),
            m_swapChain.m_backbufferIndex,
            m_swapChain.m_rtvHeap.GetDescSize()
        );
        m_commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

        // record commands
        constexpr Array<float, 4> clearColor{ 0.0f, 0.2f, 0.4f, 1.0f };
        m_commandList->ClearRenderTargetView(rtvHandle, clearColor.data(), 0, nullptr);
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        m_commandList->IASetIndexBuffer(&m_indexBufferView);
        m_commandList->DrawIndexedInstanced(
            static_cast<u32>(boxIndices.size()),
            1, 0, 0, 0
        );

        _ImGuiDraw();

        // present backbuffer
        resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                m_swapChain.m_buffers[m_swapChain.m_backbufferIndex].Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PRESENT
        );

        m_commandList->ResourceBarrier(1, &resourceBarrier);
        WIN_CHECK(m_commandList->Close());
    }

    void Renderer::_FlushCommandQueue()
    {
        m_fenceValue++;
        
        // Schedule a Signal command in the queue.
        WIN_CHECK(m_commandQueue->Signal(m_fence.Get(), m_fenceValue));

        // Wait until cmd queue reaches fence 
        WIN_CHECK(m_fence->SetEventOnCompletion(
            m_fenceValue, 
            m_fenceEvent.Get()
        ));

        ILL_VERIFY(WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, false) == WAIT_OBJECT_0);
        
        //DWORD res{ WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, false) };

        //ILL_ASSERT(res == WAIT_OBJECT_0);
    }

    void Renderer::_EndFrame()
    {
        // Schedule a Signal command in the queue.
        WIN_CHECK(m_commandQueue->Signal(m_fence.Get(), ++m_fenceValue));
        m_pCurrFrameRes->m_fence = m_fenceValue;

        // Update the frame index.
        m_swapChain.UpdateBackbufferIndex();
    }
}