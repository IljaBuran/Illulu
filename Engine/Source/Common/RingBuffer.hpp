#pragma once

#include "Common.hpp"

#include "Array.hpp"

// TODO: this needs further functionaly, i'll implement it later (maybe hehe)

template<typename T, std::uint32_t size>
class RingBuffer
{
public:

	void Add(T&& element) noexcept
	{
		m_data[m_writeIndex] = std::move(element);

		m_writeIndex = (m_writeIndex + 1) % size;

		if (m_count < size)
		{
			m_count++;
		}
	}
	
	[[nodiscard]]
	T& operator[](std::uint32_t index) noexcept
	{
		ILL_ASSERT(index < m_count);

		const std::uint32_t m_oldest{ m_count == size ? m_writeIndex : 0 };

		return m_data[(static_cast<std::uint64_t>(m_oldest) + index) % size];
	}

	[[nodiscard]]
	const T& operator[](std::uint32_t index) const noexcept
	{
		ILL_ASSERT(index < m_count);

		const std::uint32_t m_oldest{ m_count == size ? m_writeIndex : 0 };

		return m_data[(static_cast<std::uint64_t>(m_oldest) + index) % size];
	}

	[[nodiscard]]
	std::uint32_t Size() const noexcept
	{
		return m_count;
	}

private:

	std::array<T, size> m_data;
	std::uint32_t m_writeIndex{};
	std::uint32_t m_count{};
};