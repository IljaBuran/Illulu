#pragma once

#include "Common/Types.h"

#include <type_traits>
#include <stdexcept>

namespace Illulu
{
	template<typename First, typename... Ts>
	inline void ASSERT(const tchar* message, First&& first, Ts&&... args) 
	{
	#ifdef _DEBUG
		static_assert((std::convertible_to<std::remove_cvref_t<First>, bool>), 
			"All arguments of ASSERT need to be convertible to bool");
		static_assert((std::convertible_to<std::remove_cvref_t<Ts>, bool> && ...), 
			"All arguments of ASSERT need to be convertible to bool");

		if (!(static_cast<bool>(first) && ... && static_cast<bool>(args)))
		{
			// LOG
		}
		
	#endif
	}
}