#include "FrameResource.hpp"

namespace Illulu
{
    void FrameResource::Create(ID3D12DeviceIll* device, u32 passCount)
    {
        m_commandAllocator.Create(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
        m_passCB.Create(device, passCount, true);
    }

    FrameResource::~FrameResource() {}
}
