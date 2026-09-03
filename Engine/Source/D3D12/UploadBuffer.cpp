#include "D3D12/UploadBuffer.hpp"

#include <Math/Math.hpp>

namespace Illulu::D3D12
{
    template<typename T>
    UploadBuffer<T>::UploadBuffer(ID3D12DeviceIll* const device, u64 elementCount, bool isConstantBuffer)
        : m_elementCount(elementCount), m_elementByteSize(sizeof(T))
    {
        if (isConstantBuffer)
        {
            m_elementByteSize = AlignUp<D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, m_elementByteSize>();
        }

        auto properties{CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)};
        auto description{CD3DX12_RESOURCE_DESC::Buffer(m_elementByteSize * elementCount)};

        WIN_CHECK(device->CreateCommittedResource(
            &properties,
            D3D12_HEAP_FLAG_NONE,
            &description,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_uploadBuffer)
        ));

        auto range{CD3DX12_RANGE(0, 0)};
        WIN_CHECK(m_uploadBuffer->Map(0, &range, reinterpret_cast<void**>(&m_mappedData)));
    }

    template<typename T>
    UploadBuffer<T>::~UploadBuffer()
    {
        if (m_uploadBuffer)
        {
            m_uploadBuffer->Unmap(0, nullptr);
        }
        m_mappedData = nullptr;
    }
    template<typename T>
    void UploadBuffer<T>::CopyData(i32 elementIndex, const T& data)
    {
        ILL_ASSERT(m_uploadBuffer);
        ILL_ASSERT(elementIndex < m_elementCount);
        
        std::memcpy(
            &m_mappedData[elementIndex * m_elementByteSize],
            &data,
            sizeof(T)
        );
    }
    template<typename T>
    void UploadBuffer<T>::CopyData(const T* data, u32 count)
    {
        ILL_ASSERT(m_uploadBuffer);
        ILL_ASSERT(m_elementByteSize == sizeof(T));

        std::memcpy(
            m_mappedData,
            data,
            count * m_elementByteSize
        );
    }
}