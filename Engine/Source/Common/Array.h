#pragma once

#ifdef ILL_STL
#include <array>

template<typename T, std::uint64_t size>
using Array= std::array<T, size>;
#endif