#pragma once

#if defined(ILL_STL)
#include <unordered_map>

template<
	typename Key,
	typename T,
	typename Hash = std::hash<Key>,
	typename KeyEqual = std::equal_to<Key>,
	typename Allocator = std::allocator<std::pair<const Key, T>>
>
using HashMap = std::unordered_map<
	Key,
	T,
	Hash,
	KeyEqual,
	Allocator
>;
#endif