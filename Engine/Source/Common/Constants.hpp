#pragma once

#include <cstdint>
#include <limits>

namespace Illulu
{
	constexpr std::uint64_t U8_MAX = std::numeric_limits<std::uint8_t>::max();
	constexpr std::uint64_t U16_MAX = std::numeric_limits<std::uint16_t>::max();
	constexpr std::uint64_t U32_MAX = std::numeric_limits<std::uint32_t>::max();
	constexpr std::uint64_t U64_MAX = std::numeric_limits<std::uint64_t>::max();

	constexpr std::int64_t I8_MAX = std::numeric_limits<std::int8_t>::max();
	constexpr std::int64_t I16_MAX = std::numeric_limits<std::int16_t>::max();
	constexpr std::int64_t I32_MAX = std::numeric_limits<std::int32_t>::max();
	constexpr std::int64_t I64_MAX = std::numeric_limits<std::int64_t>::max();
}