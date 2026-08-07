#include "Common.hpp"

#include "DX12.hpp"

#include "array.hpp"

namespace Illulu::D3D12
{
    class CommandListAllocator
    {
    public:

        CommandListAllocator() = default;

        void Create(ID3D12DeviceIll* const device, D3D12_COMMAND_LIST_TYPE type);

        ID3D12CommandAllocator* const GetAllocPtr()
        {
            ILL_ASSERT(m_d3d12commandAllocator);

            return m_d3d12commandAllocator.Get();
        }

        ID3D12CommandAllocator* const operator->()
        {
            return GetAllocPtr();
        }

        operator ID3D12CommandAllocator* const()
        {
            return GetAllocPtr();
        }

        CommandListAllocator(const CommandListAllocator&) = delete;
        CommandListAllocator(CommandListAllocator&&)      = delete;
        void operator=(const CommandListAllocator&)       = delete;
        void operator=(CommandListAllocator&&)            = delete;

    private:

        ComPtr<ID3D12CommandAllocator> m_d3d12commandAllocator;
    };
}