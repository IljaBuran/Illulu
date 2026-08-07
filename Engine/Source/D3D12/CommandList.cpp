#include "CommandList.hpp"

namespace Illulu::D3D12
{
    void CommandList::Create(
        ID3D12DeviceIll* const device, 
        D3D12_COMMAND_LIST_TYPE type, 
        ID3D12CommandAllocator* const allocator, 
        ID3D12PipelineState* const initialState
    )
    {
        ILL_ASSERT(device && allocator);
        
        WIN_CHECK(device->CreateCommandList(
            0 /* FOR SINGLE ADAPTER PROGRAMS IT'S OK TO SET THIS TO 0 */,
            type,
            allocator,
            initialState,
            IID_PPV_ARGS(&m_d3d12CommandList)
        ));
    }
}

