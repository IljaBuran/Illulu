#pragma once

#if defined(ILL_STL)
#include <queue>

template<
    class T,
    class Container = std::deque<T>
>
using Queue = std::queue<T, Container>;
#endif // defined(ILL_STL)