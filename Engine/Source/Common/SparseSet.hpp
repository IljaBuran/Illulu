#pragma once

#include "Common.hpp"

#include <memory>
#include "Vector.hpp"

using namespace Illulu;

template <typename T>
class SparseSet
{
public:
    
    SparseSet(u32 capacity)
        : m_capacity{ capacity },
          m_sparse(std::make_unique<u32[]>(capacity)) {}

    // returns true if it was added
    // returns false if it was already in the set
    bool Add(u32 id) noexcept
    {
        
        ILL_ASSERT(id < m_capacity);

        // TODO::!!!!

        if (m_dense.size() >= m_capacity)
        {
            WARNING(L"[SparseSet] Capacity exceeded, enterying undefined state");
        }

        if (!Contains(id))
        {
            m_sparse[id] = static_cast<u32>(m_dense.size());
            m_dense.emplace_back(id);

            return true;
        }

        return false;
    }

    // returns true if it was deleted successfully
    // returns false if it was not deleted (element wasn't in the set)
    bool Remove(u32 id) noexcept
    {
        ILL_ASSERT(id < m_capacity);

        const u32 indexInDense{ m_sparse[id] };

        if (Contains(id))
        {
            const u32 last{ m_dense.back() };
            m_dense[indexInDense] = last;
            m_dense.pop_back();

            m_sparse[last] = indexInDense;

            return true;
        }

        return false;
    }

    bool Contains(u32 id) const noexcept
    {
        ILL_ASSERT(id < m_capacity);

        const u32 indexInDense{ m_sparse[id] };

        return (indexInDense < m_dense.size() && id == m_dense[indexInDense]);
    }

    T& Get(u32 id) noexcept
    {
        ILL_ASSERT(id < m_capacity);
        ILL_ASSERT(Contains(id));

        return m_data[id];
    }

    const T& Get(u32 id) const noexcept
    {
        ILL_ASSERT(id < m_capacity);
        ILL_ASSERT(Contains(id));

        return m_data[id];
    }

    void Clear()
    {
        m_dense.clear();
        m_data.clear();
    }

    // iterator
    // we don't want to let user change the inner structure from outside
    using iterator = Vector<u32>::const_iterator;
    using const_iterator = Vector<u32>::const_iterator;

    iterator begin() noexcept { return m_dense.cbegin(); }
    iterator end()   noexcept { return m_dense.cend(); }

    const_iterator begin()  const noexcept { return m_dense.cbegin(); }
    const_iterator end()    const noexcept { return m_dense.cend(); }
    const_iterator cbegin() const noexcept { return m_dense.cbegin(); }
    const_iterator cend()   const noexcept { return m_dense.cend(); }

private:

    u32 m_capacity{ 0 };

    std::unique_ptr<uint32_t[]> m_sparse{};
    Vector<u32> m_dense{};
    Vector<T>   m_data{};
};