#pragma once

#include "Common.hpp"

#include <memory>

#include "Math/Math.hpp"

#include "Array.hpp"
#include "DX12.hpp"

//#include "Core/ShaderManager.hpp"
#include "Util/FileWatcher.hpp"

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

#include "D3D12/DescriptorUtil.hpp"
#include "D3D12/InfoQueue.hpp"
#include "D3D12/Factory.hpp"
#include "D3D12/Device.hpp"
#include "D3D12/CommandQueue.hpp"
#include "D3D12/CommandListAllocator.hpp"
#include "D3D12/SwapChain.hpp"
#include "D3D12/CommandList.hpp"
#include "D3D12/Shader.hpp"
#include "D3D12/FrameResource.hpp"

#include "GraphicsMemory.h"

namespace Illulu
{
    class Renderer
    {
    public: /* Public functions */

        void OnInitialize(HWND hWnd);
        void OnUpdate();
        void OnRender();
        void OnShutdown();

        void RecompileShader();

        void UpdateRenderTargetSize(i32 newWidth, i32 newHeight);

    private: /* Private functions */

        /* ImGui */
        void _ImGuiInit(HWND hWnd);
        void _ImGuiStartFrame();
        void _ImGuiDraw();
        void _ImGuiShutdown();


        void _BuildFrameResources();
        void _FeedCommandList();
        void _FlushCommandQueue();
        void _EndFrame();

    public:

        // TEMP SHIAT
        i32 m_deltaX{};
        i32 m_deltaY{};

    private: /* Private variables */

        //ShaderManager       m_shaderManager{};
        FileWatcher         m_fileWatcher;
        
        D3D12::Device       m_device{};
        D3D12::CommandQueue m_commandQueue{};
        D3D12::SwapChain    m_swapChain;
        D3D12::CommandList  m_commandList{};

        D3D12_VIEWPORT m_viewport{};
        D3D12_RECT     m_scissorRect{};

        ComPtr<ID3D12RootSignature> m_rootSignature{};
        ComPtr<ID3D12PipelineState> m_pipelineState{};

        DescriptorHeap m_dsvHeap{};
        CbvSrvUavHeap  m_cbvSrvUavHeap{};

        /* synchronization */
        ComPtr<ID3D12FenceIll> m_fence{};
        Event                  m_fenceEvent{};
        u64                    m_fenceValue{};
        
        /* app resources */ 

        // vertex and index data
        ComPtr<ID3D12Resource> m_vertexIndexBufferGPU{};
        ComPtr<ID3D12Resource> m_uploadBuffer{};
        byte*                  m_mappedUploadBuffer{ nullptr };
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
        D3D12_INDEX_BUFFER_VIEW m_indexBufferView{};

        // per frame constants
        static constexpr u32 numFrameResources{ 3 };
        Array<FrameResource, numFrameResources> m_frameResources{};
        u8 m_currFrameResIdx{ 0 };
        FrameResource* m_pCurrFrameRes{ nullptr };

        // per object
        std::unique_ptr<GraphicsMemory> m_linearAllocator;
        GraphicsResource m_objectAllocation;

        // shaders
        D3D12::Shader m_shader;

        /* render target info */
        u32 m_renderTargetWidth{};
        u32 m_renderTargetHeight{};

        bool m_initialized{ false };

        /* IMGUI's console */

        ImGUIConsole m_imGuiConsole;
    };
}