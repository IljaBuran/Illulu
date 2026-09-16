#pragma once

#include <cstdint>
#include <numeric>

#include <cassert>

#if !defined(FORCEINLINE)
#define FORCEINLINE __forceinline
#endif

template<typename T>
concept HasLessComparator = requires(T t)
{
    { t < t } -> std::convertible_to<bool>;
};

[[nodiscard]] FORCEINLINE constexpr
bool IsPowerOf2(std::uint64_t value) noexcept
{
    return (value & (value - 1)) == 0;
}

[[nodiscard]] FORCEINLINE constexpr
std::uint64_t AlignUp(std::uint64_t value, std::uint64_t alignment) noexcept
{
    // IMPORTANT CHECK: non zero alignment / needs to be power of 2
    assert(alignment != 0 && IsPowerOf2(alignment));
    const std::uint64_t mask = alignment - 1;
    assert(value <= std::numeric_limits<std::uint64_t>::max() - mask);
    return (value + mask) & ~mask;
}

template<HasLessComparator T>
[[nodiscard]] FORCEINLINE constexpr 
T Clamp(const T& value, const T& min, const T& max)
{
    assert(min < max);

    if (value < min)
        return min;
    if (max < value)
        return max;
    return value;
}