#pragma once

#include "Common.h"

#include "DX12.h"

#include <queue>
#include <unordered_set>

namespace Illulu
{
    class DescriptorHeap
    {
    public:

        DescriptorHeap() = default;

        void Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE desc_heap_type, u32 capacity);

        ID3D12DescriptorHeap* GetHeap() const noexcept;

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(u32 index) const noexcept;
        CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(u32 index) const noexcept;
        
        bool IsInitialized() const noexcept;

    private:
        ComPtr<ID3D12DescriptorHeap> m_heap;
        u32 m_descriptorSize = 0;
    };

    class CbvSrvUavHeap : public DescriptorHeap
    {
    public:

        CbvSrvUavHeap() = default;

        void Initialize(ID3D12Device* device, u32 capacity);

        u32 GetNextFreeIndex();
        void ReleaseIndex(u32 index);

    private:

        std::queue<u32> m_freeIndices;

        // only for validation, this can be only debug 
        std::unordered_set<u32> m_usedIndices;
    };
}
