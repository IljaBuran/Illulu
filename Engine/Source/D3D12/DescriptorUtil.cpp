#include "DescriptorUtil.hpp"

namespace Illulu
{
    void DescriptorHeap::Create(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE descHeapType, u32 capacity)
    {
        ILL_ASSERT(!m_heap);
        ILL_ASSERT(device);

        D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc
        {
            .Type = descHeapType,
            .NumDescriptors = capacity,
            .Flags = (descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || descHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) ?
            D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = 0
        };

        WIN_CHECK(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(m_heap.GetAddressOf())));

        m_descriptorSize = device->GetDescriptorHandleIncrementSize(descHeapType);
    }

    ID3D12DescriptorHeap* DescriptorHeap::GetHeap() const noexcept
    {
        ILL_ASSERT(m_heap);

        return m_heap.Get();
    }

    u32 DescriptorHeap::GetDescSize() const noexcept
    {
        ILL_ASSERT(m_heap);

        return m_descriptorSize;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCpuHandle(u32 index) const noexcept
    {
        ILL_ASSERT(m_heap);

        CD3DX12_CPU_DESCRIPTOR_HANDLE hCpu(m_heap->GetCPUDescriptorHandleForHeapStart());
        hCpu.Offset(index, m_descriptorSize);
        return hCpu;
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGpuHandle(u32 index) const noexcept
    {
        ILL_ASSERT(m_heap);
        CD3DX12_GPU_DESCRIPTOR_HANDLE hGpu(m_heap->GetGPUDescriptorHandleForHeapStart());

        hGpu.Offset(index, m_descriptorSize);
        return hGpu;
    }

    void CbvSrvUavHeap::Initialize(ID3D12Device* device, u32 capacity)
    {
        ILL_ASSERT(device && capacity > 0);
        DescriptorHeap::Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, capacity);

        for (u32 i = 0; i < capacity; ++i)
            m_freeIndices.push(i);

        m_usedIndices.clear();
    }

    u32 CbvSrvUavHeap::GetNextFreeIndex()
    {
        ILL_ASSERT(m_heap);
        ILL_ASSERT(!m_freeIndices.empty());

        const i32 index = m_freeIndices.front();

        m_usedIndices.insert(index);

        m_freeIndices.pop();
        return index;
    }

    void CbvSrvUavHeap::ReleaseIndex(u32 index)
    {
        ILL_ASSERT(m_heap);
        auto it = m_usedIndices.find(index);
        ILL_ASSERT(it != m_usedIndices.end());

        m_usedIndices.erase(it);
        m_freeIndices.push(index);
    }
}
