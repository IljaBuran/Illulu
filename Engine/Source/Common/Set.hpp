#pragma once

#if defined(ILL_STL)

#include <set>

template<
    class Key,
    class Compare = std::less<Key>,
    class Allocator = std::allocator<Key>
>

using Set = std::set<Key, Compare, Allocator>;

#endif // defined(ILL_STL)