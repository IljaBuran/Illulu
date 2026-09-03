#pragma once

#include "Common.hpp"

#include "DX12.hpp"

#include "Queue.hpp"
#include "HashSet.hpp"

namespace Illulu
{
    class DescriptorHeap
    {
    public:

        DescriptorHeap() = default;

        void Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE desc_heap_type, u32 capacity);
        
        [[nodiscard]]
        u32 GetDescSize() const noexcept;
        [[nodiscard]]
        u32 GetIndex(D3D12_CPU_DESCRIPTOR_HANDLE handle) const;

        ID3D12DescriptorHeap* const GetHeapPtr()
        {
            ILL_ASSERT(m_heap);
            return m_heap.Get();
        }

        operator ID3D12DescriptorHeap* const()
        {
            return GetHeapPtr();
        }

        ID3D12DescriptorHeap* const operator->()
        {
            return GetHeapPtr();
        }

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(u32 index) const noexcept;
        CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(u32 index) const noexcept;

    protected:

        ComPtr<ID3D12DescriptorHeap> m_heap{};
        u32 m_descriptorSize{};
    };

    class CbvSrvUavHeap : public DescriptorHeap
    {
    public:

        CbvSrvUavHeap() = default;

        // todo: this looks like bad design, 2 initialize functions, i don't like it
        void Initialize(ID3D12Device* device, u32 capacity);

        u32 GetNextFreeIndex();
        void ReleaseIndex(u32 index);

    private:

        Queue<u32> m_freeIndices;

        // only for validation, this can be only debug 
        HashSet<u32> m_usedIndices;
    };
}
