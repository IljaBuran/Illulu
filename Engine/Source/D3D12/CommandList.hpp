#include "Common.hpp"

#include "DX12.hpp"

namespace Illulu::D3D12
{
    class CommandList
    {
    public:

        CommandList() = default;

        void Create(
            ID3D12DeviceIll* const device, 
            D3D12_COMMAND_LIST_TYPE type, 
            ID3D12CommandAllocator* const allocator, 
            ID3D12PipelineState* const initialState = nullptr
        );

        ID3D12GraphicsCommandListIll* const GetCommandListPtr()
        {
            ILL_ASSERT(m_d3d12CommandList);
            
            return m_d3d12CommandList.Get();
        }

        ID3D12GraphicsCommandListIll* const operator->()
        {
            return GetCommandListPtr();
        }

        operator ID3D12GraphicsCommandListIll* const()
        {
            return GetCommandListPtr();
        }

        CommandList(const CommandList&) = delete;
        CommandList(CommandList&&) = delete;
        void operator=(const CommandList&) = delete;
        void operator=(CommandList&&) = delete;

    private:

        ComPtr <ID3D12GraphicsCommandListIll> m_d3d12CommandList;
    };
}