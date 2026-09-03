#include <array>
#include <vector>

#include <cstdint>

#include <limits>

#include <cassert>

template<std::uint32_t Capacity>
class SparseSet
{
public:

    // returns true if it was added
    // returns false if it was already in the set
    bool Add(std::uint32_t id) noexcept
    {
        assert(id < Capacity);
        assert(m_dense.size() < Capacity);

        if (!Contains(id))
        {
            m_sparse[id] = static_cast<std::uint32_t>(m_dense.size());
            m_dense.emplace_back(id);

            return true;
        }

        return false;
    }

    // returns true if it was deleted successfully
    // returns false if it was not deleted (element wasn't in the set)
    bool Remove(std::uint32_t id) noexcept
    {
        assert(id < Capacity);

        const std::uint32_t indexInDense{m_sparse[id]};

        if (Contains(id))
        {
            const std::uint32_t last{ m_dense.back() };
            m_dense[indexInDense] = last;
            m_dense.pop_back();

            m_sparse[last] = indexInDense;

            return true;
        }

        return false;
    }

    bool Contains(std::uint32_t id) const noexcept
    {
        assert(id < Capacity);
        
        const std::uint32_t indexInDense{ m_sparse[id] };

        return (indexInDense < m_dense.size() && id == m_dense[indexInDense]);
    }

    void Clear()
    {
        m_dense.clear();
    }

    // iterator
    // we don't want to let user change the inner structure from outside
    using iterator = std::vector<std::uint32_t>::const_iterator;
    using const_iterator = std::vector<std::uint32_t>::const_iterator;

    iterator begin() noexcept { return m_dense.cbegin(); }
    iterator end()   noexcept { return m_dense.cend(); }

    const_iterator begin()  const noexcept { return m_dense.cbegin(); }
    const_iterator end()    const noexcept { return m_dense.cend(); }
    const_iterator cbegin() const noexcept { return m_dense.cbegin(); }
    const_iterator cend()   const noexcept { return m_dense.cend(); }

private:

    std::array<std::uint32_t, Capacity> m_sparse{};
    std::vector<std::uint32_t> m_dense{};
};
