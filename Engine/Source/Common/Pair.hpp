#pragma once

#if defined(ILL_STL)
#include <utility>

template<typename First, typename Second>
using Pair = std::pair<First, Second>;
#endif