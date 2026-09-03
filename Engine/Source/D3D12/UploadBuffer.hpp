#pragma once
#include "Common.hpp"

#include "DX12.hpp"

#include "D3D12/DescriptorUtil.hpp"

namespace Illulu::D3D12
{
    template<typename T>
    class UploadBuffer
    {
    public:

        UploadBuffer(ID3D12DeviceIll* device, u64 elementCount, bool isConstantBuffer);

        ~UploadBuffer();

        void CopyData(i32 elementIndex, const T& data);
        void CopyData(const T* data, u32 count);

        ID3D12Resource* GetResourcePtr()
        {
            ILL_ASSERT(m_uploadBuffer);
            return m_uploadBuffer.Get();
        }

        UploadBuffer(const UploadBuffer&) = delete;
        UploadBuffer(UploadBuffer&&) = delete;
        void operator=(const UploadBuffer&) = delete;
        void operator=(UploadBuffer&&) = delete;

    private:

        ComPtr<ID3D12Resource> m_uploadBuffer;
        byte* m_mappedData{nullptr};

        u32 m_elementCount{0};
        u32 m_elementByteSize{0};
        bool m_isConstantBuffer{false};
    };
}