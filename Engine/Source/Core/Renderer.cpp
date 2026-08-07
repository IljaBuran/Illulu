#include "Renderer.hpp"

#include "Common.hpp"
#include "WindowsMin.hpp"

#include <cmath>
#include "Vector.hpp"
#include "String.hpp"

#include "Core/Filesystem.hpp"

#include "D3D12/Util.hpp"

#include "Data.hpp"


namespace Illulu
{
    void Renderer::OnInitialize(HWND hWnd)
    {
        INFO(L"*** [DX12] Initialization start ***");

        // this should be already initialized correctly
        ILL_ASSERT(m_renderTargetHeight && m_renderTargetWidth);

        /* Initialize scissor rect and viewport */
        {
            m_viewport =
            {
                .TopLeftX{0.0f},
                .TopLeftY{0.0f},
                .Width{static_cast<f32>(m_renderTargetWidth)},
                .Height{static_cast<f32>(m_renderTargetHeight)},
                .MinDepth{0.0f},
                .MaxDepth{1.0f}
            };

            m_scissorRect =
            {
                .left{0},
                .top{0},
                .right{static_cast<i32>(m_renderTargetWidth)},
                .bottom{static_cast<i32>(m_renderTargetHeight)},
            };
        }

        D3D12::Factory factory{};
        m_device.Initialize(factory);

        m_commandQueue.Create(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);

        /* Create CBV's descriptor heap */
        m_cbvHeap.Initialize(m_device, 2);

        /* Create DSV's descriptor heap */
        m_dsvHeap.Create(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, FRAMEBUFFER_COUNT);

        /* Initialize swap chain */
        m_swapChain.Create(factory, m_device, m_commandQueue, hWnd, m_renderTargetWidth, m_renderTargetHeight);

        /* Create command list allocators */
        for (u32 i = 0; i < FRAMEBUFFER_COUNT; i++)
        {
            m_commandListAllocators[i].Create(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
        }

        /* Create a root signature consisting of a descriptor table with single CBV */
        {
            D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData
            {
                .HighestVersion{D3D_ROOT_SIGNATURE_VERSION_1_1}
            };

            Array<CD3DX12_DESCRIPTOR_RANGE1, 1> ranges{};
            Array<CD3DX12_ROOT_PARAMETER1, 1> rootParameters{};

            {
                constexpr u32 descriptorCount{2};
                constexpr u32 shaderRegister{0};
                constexpr u32 registerSpace{0};

                ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, descriptorCount, shaderRegister, registerSpace, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
            }
            rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

            D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

            CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
            rootSignatureDesc.Init_1_1(static_cast<u32>(rootParameters.size()), rootParameters.data(), 0, nullptr, rootSignatureFlags);

            ComPtr<ID3DBlob> signature{};
            ComPtr<ID3DBlob> error{};

            WIN_CHECK(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));

            WIN_CHECK(m_device->CreateRootSignature(
                0,
                signature->GetBufferPointer(),
                signature->GetBufferSize(),
                IID_PPV_ARGS(&m_rootSignature)
            ));
        }

        /* Create pipeline state (includes compiling and loading shaders) */
        {
            Vector<byte> vertexShaderData = Filesystem::ReadBinaryBlobFromFile(L"Basic_VSMain.cso");
            Vector<byte> pixelShaderData = Filesystem::ReadBinaryBlobFromFile(L"Basic_PSMain.cso");

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
                .InputLayout{ColorVertex::GetInputLayoutDesc()},
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
            m_commandList.Create(
                m_device,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_commandListAllocators[m_swapChain.m_backbufferIndex],
                m_pipelineState.Get()
            );

            // command list is created in recording state -> close it
            WIN_CHECK(m_commandList->Close());
        }

        /* Create synchronization objects */
        {
            WIN_CHECK(m_device->CreateFence(
                m_fenceValues[m_swapChain.m_backbufferIndex],
                D3D12_FENCE_FLAG_NONE,
                IID_PPV_ARGS(&m_fence)
            ));

            m_fenceValues[m_swapChain.m_backbufferIndex]++;

            // create event handle
            m_fenceEvent.Attach(CreateEvent(nullptr, false, false, nullptr));
            ILL_ASSERT(m_fenceEvent.IsValid());
        }


        u32 totalSizeNeeded{sizeof(boxVertices) + sizeof(boxIndices)};
        // Create default heap resource, we'll copy the vertices and indices here from upload buffer
        {
            auto heapProps{CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)};
            auto resDesc{CD3DX12_RESOURCE_DESC::Buffer(totalSizeNeeded)};

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
            auto heapProps{CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)};
            auto resDesc{CD3DX12_RESOURCE_DESC::Buffer(totalSizeNeeded)};

            WIN_CHECK(m_device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &resDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_uploadBuffer)
            ));

            auto mapRange{CD3DX12_RANGE(0, 0)};
            m_uploadBuffer->Map(0, &mapRange, reinterpret_cast<void**>(&m_mappedUploadBuffer));

            // copy vertices and indices into upload buffer
            std::memcpy(m_mappedUploadBuffer, boxVertices.data(), sizeof(boxVertices));
            std::memcpy(m_mappedUploadBuffer + sizeof(boxVertices), boxIndices.data(), sizeof(boxIndices));

            m_uploadBuffer->Unmap(0, nullptr);
        }

        // VERY IMPORTANT HERE:
        WIN_CHECK(m_commandList->Reset(m_commandListAllocators[m_swapChain.m_backbufferIndex], nullptr));


        // transition default buffer to COPY_DEST, copy and then transition back
        {
            D3D12_RESOURCE_BARRIER toCopyDest
            {
                CD3DX12_RESOURCE_BARRIER::Transition(
                    m_vertexIndexBufferGPU.Get(),
                    D3D12_RESOURCE_STATE_COMMON,
                    D3D12_RESOURCE_STATE_COPY_DEST)
            };

            m_commandList->ResourceBarrier(1, &toCopyDest);

            m_commandList->CopyBufferRegion(
                m_vertexIndexBufferGPU.Get(), 0,
                m_uploadBuffer.Get(), 0,
                totalSizeNeeded
            );

            D3D12_RESOURCE_BARRIER toGenericRead
            {
                CD3DX12_RESOURCE_BARRIER::Transition(
                    m_vertexIndexBufferGPU.Get(),
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    D3D12_RESOURCE_STATE_GENERIC_READ)
            };

            m_commandList->ResourceBarrier(1, &toGenericRead);
        }

        // execute the copy from upload to default
        {
            WIN_CHECK(m_commandList->Close());

            Array<ID3D12CommandList*, 1> lists{m_commandList.GetCommandListPtr()};
            m_commandQueue->ExecuteCommandLists(static_cast<u32>(lists.size()), lists.data());

            _WaitForGpu();
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

        /* create constant buffers */
        {
            // PER OBJECT

            constexpr u32 objCbSize = static_cast<u32>(AlignUp<D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(sizeof(cbPerObject)));

            auto heapProps{CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)};
            auto resDesc{CD3DX12_RESOURCE_DESC::Buffer(objCbSize)};

            WIN_CHECK(m_device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &resDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_perObjectUploadBuffer)
            ));

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc
            {
                .BufferLocation{m_perObjectUploadBuffer->GetGPUVirtualAddress()},
                .SizeInBytes{objCbSize}
            };

            m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap.GetCpuHandle(0));

            auto mapRange{CD3DX12_RANGE(0, 0)};
            m_perObjectUploadBuffer->Map(0, &mapRange, reinterpret_cast<void**>(&m_pPerObjectMapped));

            // PER PASS
            constexpr u32 passCbSize = static_cast<u32>(AlignUp<D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(sizeof(cbPerPass)));
            resDesc.Width = passCbSize;

            WIN_CHECK(m_device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &resDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_perPassUploadBuffer)
            ));

            cbvDesc =
            {
                .BufferLocation{m_perPassUploadBuffer->GetGPUVirtualAddress()},
                .SizeInBytes{passCbSize}
            };

            m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap.GetCpuHandle(1));
            m_perPassUploadBuffer->Map(0, &mapRange, reinterpret_cast<void**>(&m_pPerPassMapped));
        }


        m_initialized = true;
        INFO(L"*** [DX12] Initialization successful ***");
    }

    void Renderer::OnUpdate()
    {
        using namespace DirectX;

        // --- per object: world matrix ---
        static float angle = 0.0f;
        angle += 0.01f;

        XMMATRIX world = XMMatrixRotationY(angle);
        cbPerObject objData{};
        XMStoreFloat4x4(&objData.M, XMMatrixTranspose(world)); // HLSL expects column-major by default, transpose row-major XMMatrix
        memcpy(m_pPerObjectMapped, &objData, sizeof(objData));

        // --- per pass: view-projection matrix ---
        XMVECTOR eye = XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f);
        XMVECTOR target = XMVectorZero();
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMMATRIX view = XMMatrixLookAtLH(eye, target, up);

        float aspect = static_cast<float>(m_renderTargetWidth) / static_cast<float>(m_renderTargetHeight);
        XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.1f, 100.0f);

        cbPerPass passData{};
        XMStoreFloat4x4(&passData.VP, XMMatrixTranspose(view * proj));
        memcpy(m_pPerPassMapped, &passData, sizeof(passData));
    }

    void Renderer::OnRender()
    {
        _FeedCommandList();

        // Execute the command list.
        const Array<ID3D12CommandList*, 1> commandLists = {m_commandList};
        m_commandQueue->ExecuteCommandLists(static_cast<u32>(commandLists.size()), commandLists.data());
        // Present the frame.
        WIN_CHECK(m_swapChain->Present(1, 0));

        _EndFrame();
    }

    void Renderer::OnShutdown()
    {
        _WaitForGpu();

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
            .TopLeftX{0.0f},
            .TopLeftY{0.0f},
            .Width{static_cast<f32>(m_renderTargetWidth)},
            .Height{static_cast<f32>(m_renderTargetHeight)},
            .MinDepth{0.0f},
            .MaxDepth{1.0f}
        };

        m_scissorRect =
        {
            .left{0},
            .top{0},
            .right{static_cast<i32>(m_renderTargetWidth)},
            .bottom{static_cast<i32>(m_renderTargetHeight)}
        };

        const u64 fenceValue{m_fenceValues[m_swapChain.m_backbufferIndex]};

        for (u32 i{0}; i < FRAMEBUFFER_COUNT; i++)
        {
            m_fenceValues[i] = fenceValue;
        }

        m_swapChain.Resize(m_device, newWidth, newHeight);

        INFO(L"[DX12] Render target resized");
    }

    void Renderer::_FeedCommandList()
    {
        // reset command list allocator -> this can only happen when command list associated with the allocator has finished
        WIN_CHECK(m_commandListAllocators[m_swapChain.m_backbufferIndex]->Reset());

        // command list can be reset immediately after calling ExecuteCommandLists on it
        WIN_CHECK(m_commandList->Reset(m_commandListAllocators[m_swapChain.m_backbufferIndex], m_pipelineState.Get()));

        // setting states
        m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

        Array<ID3D12DescriptorHeap*, 1> ppHeaps{m_cbvHeap.GetHeap()};
        m_commandList->SetDescriptorHeaps(static_cast<u32>(ppHeaps.size()), ppHeaps.data());

        m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap.GetGpuHandle(0));
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
        constexpr Array<float, 4> clearColor{0.0f, 0.2f, 0.4f, 1.0f};
        m_commandList->ClearRenderTargetView(rtvHandle, clearColor.data(), 0, nullptr);
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        m_commandList->IASetIndexBuffer(&m_indexBufferView);
        m_commandList->DrawIndexedInstanced(
            static_cast<u32>(boxIndices.size()),
            1, 0, 0, 0
        );

        // present backbuffer
        resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                m_swapChain.m_buffers[m_swapChain.m_backbufferIndex].Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PRESENT
        );

        m_commandList->ResourceBarrier(1, &resourceBarrier);
        WIN_CHECK(m_commandList->Close());
    }

    void Renderer::_WaitForGpu()
    {
        // Schedule a Signal command in the queue.
        WIN_CHECK(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_swapChain.m_backbufferIndex]));

        // Wait until the fence has been processed.
        WIN_CHECK(m_fence->SetEventOnCompletion(m_fenceValues[m_swapChain.m_backbufferIndex], m_fenceEvent.Get()));
        WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, false);

        // Increment the fence value for the current frame.
        m_fenceValues[m_swapChain.m_backbufferIndex]++;
    }

    void Renderer::_EndFrame()
    {
        // Schedule a Signal command in the queue.
        const u64 currentFenceValue = m_fenceValues[m_swapChain.m_backbufferIndex];
        WIN_CHECK(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

        // Update the frame index.
        m_swapChain.UpdateBackbufferIndex();

        // If the next frame is not ready to be rendered yet, wait until it is ready.
        if (m_fence->GetCompletedValue() < m_fenceValues[m_swapChain.m_backbufferIndex])
        {
            WIN_CHECK(m_fence->SetEventOnCompletion(m_fenceValues[m_swapChain.m_backbufferIndex], m_fenceEvent.Get()));
            WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, false);
        }

        // Set the fence value for the next frame.
        m_fenceValues[m_swapChain.m_backbufferIndex] = currentFenceValue + 1;
    }
}