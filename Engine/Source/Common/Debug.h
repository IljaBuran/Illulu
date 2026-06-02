#pragma once

#include "Common/Types.h"

#include <type_traits>
#include <stdexcept>

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
#endif
}
// if changing to multiliner -> make do-while loop macro
#if defined(_DEBUG)
	#define ILL_ASSERT(msg, ...) Illulu::ASSERT(msg, __VA_ARGS__)
#else
	#define ILL_ASSERT(msg, ...) (void)(__VA_ARGS__)
#endif