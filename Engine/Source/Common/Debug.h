#pragma once

#include "Types.h"

#include <type_traits>
#include <stdexcept>
#include <iterator>
#include <bitset>
#include <format>

namespace Illulu
{
#if defined(_DEBUG)
	template<typename First, typename... Ts>
	inline void ASSERT(const tchar* message, First&& first, Ts&&... args) 
	{
		static_assert((std::convertible_to<std::remove_cvref_t<First>, bool>), 
			"All evaluated arguments of ASSERT need to be convertible to bool");
		static_assert((std::convertible_to<std::remove_cvref_t<Ts>, bool> && ...), 
			"All evaluated arguments of ASSERT need to be convertible to bool");

		if (!(static_cast<bool>(first) && ... && static_cast<bool>(args)))
		{
			(void)message;
			throw std::runtime_error("Error!");
		}
		
	}

	template<typename T>
	concept Container = requires(T container)
	{
		std::begin(container);
		std::end(container);
	};

	template<Container T>
	inline string PrintContainer(const T& container) noexcept
	{
		string str = ILL_TEXT("[");
		bool first = true;

		for (const auto& value : container)
		{
			if (!first)
				str += ILL_TEXT(", ");

			str += std::format(ILL_TEXT("{}"), value);
			first = false;
		}

		str += ILL_TEXT("]\n");
		return str;
	}

	template<u64 size>
	inline string BitsetToKeys(const std::bitset<size>& bs) noexcept
	{
		string str = ILL_TEXT("[");
		bool first = true;

		for (u64 i = 0; i < size; i++)
		{
			if (bs.test(i) != true)
				continue;
			
			if (!first)
				str += ILL_TEXT(", ");

			str += std::format(ILL_TEXT("{}"), static_cast<char>(i));
			first = false;
		}

		str += ILL_TEXT("]\n");
		return str;
	}

#endif
}
// if changing to multiliner -> make do-while loop macro
#if defined(_DEBUG)
	#define ILL_ASSERT(msg, ...) Illulu::ASSERT(msg, __VA_ARGS__)
#else
	#define ILL_ASSERT(msg, ...) (void)(__VA_ARGS__)
#endif