#pragma once

#if defined(ILL_STL)
#include <vector>

template<
    class T,
    class Allocator = std::allocator<T>
>
using Vector = std::vector<T, Allocator>;
#endif // defined(ILL_STL)