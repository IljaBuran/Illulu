#pragma once

#include <cstdint>
#include <numeric>

#include <cassert>

#if !defined(FORCEINLINE)
#define FORCEINLINE __forceinline
#endif

[[nodiscard]] FORCEINLINE constexpr
bool IsPowerOf2(std::uint64_t value) noexcept
{
    return (value & (value - 1)) == 0;
}

template<std::uint64_t alignment> constexpr
[[nodiscard]] FORCEINLINE
std::uint64_t AlignUp(std::uint64_t value) noexcept
{
    // IMPORTANT CHECK: non zero alignment / needs to be power of 2
    static_assert(alignment != 0 && IsPowerOf2(alignment));

    assert(value <= std::numeric_limits<std::uint64_t>::max() - (alignment - 1));

    return (value + alignment - 1) & ~(alignment - 1);
}

[[nodiscard]] constexpr
std::uint64_t AlignUp(std::uint64_t value, std::uint64_t alignment) noexcept
{
    // IMPORTANT CHECK: non zero alignment / needs to be power of 2
    assert(alignment != 0 && IsPowerOf2(alignment));
    assert(value <= std::numeric_limits<std::uint64_t>::max() - (alignment - 1));

    const std::uint64_t mask = alignment - 1;
    return (value + mask) & ~mask;
}
