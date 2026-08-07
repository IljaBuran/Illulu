#pragma once

#include "common.hpp"

#include "DX12.hpp"

namespace Illulu::D3D12
{
    class CommandQueue
    {
    public:

        CommandQueue() = default;

        void Create(
            ID3D12DeviceIll* const device, 
            D3D12_COMMAND_LIST_TYPE type, 
            D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            D3D12_COMMAND_QUEUE_FLAGS flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
            u32 nodeMask = 0
        );

        ID3D12CommandQueue* const GetCommandQueuePtr()
        {
            return m_d3d12CommandQueue.Get();
        }

        ID3D12CommandQueueIll* const operator->()
        {
            return GetCommandQueuePtr();
        }

        operator ID3D12CommandQueueIll* const()
        {
            return GetCommandQueuePtr();
        }

        CommandQueue(const CommandQueue&) = delete;
        CommandQueue(CommandQueue&&) = delete;
        void operator=(const CommandQueue&) = delete;
        void operator=(CommandQueue&&) = delete;

    private:

        ComPtr<ID3D12CommandQueueIll> m_d3d12CommandQueue;
    };


}


