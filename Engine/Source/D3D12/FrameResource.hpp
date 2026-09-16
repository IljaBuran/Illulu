#pragma once

#include "Common.hpp"

#include "DX12.hpp"

#include "D3D12/CommandListAllocator.hpp"
#include "D3D12/UploadBuffer.hpp"

#include "Data.hpp"

namespace Illulu
{
    class FrameResource
    {
    public:

        void Create(ID3D12DeviceIll* device, u32 passCount);
        ~FrameResource();

        D3D12::CommandListAllocator m_commandAllocator;
        D3D12::UploadBuffer<PassConstants> m_passCB;
        u64 m_fence{};
    };
}
