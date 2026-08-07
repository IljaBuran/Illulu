#pragma once

#if defined(ILL_STL)
#include <bitset>

template <std::uint64_t size>
using BitSet = std::bitset<size>;
#endif

template<std::uint64_t size>
inline String BitsetToKeys(const BitSet<size>& bs) noexcept
{
	String str = L"[";
	bool first = true;

	for (std::uint64_t i{0}; i < size; i++)
	{
		if (bs.test(i) != true)
			continue;

		if (!first)
			str += L", ";

		str += std::format(L"{}", static_cast<char>(i));
		first = false;
	}

	str += L"]\n";
	return str;
}