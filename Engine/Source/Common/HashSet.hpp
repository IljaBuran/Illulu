#pragma once

#if defined(ILL_STL)
#include <unordered_set>

template<typename T>
using HashSet = std::unordered_set<T>;
#endif