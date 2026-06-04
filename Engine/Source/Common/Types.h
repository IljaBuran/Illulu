#pragma once

#include <stdint.h>
#include <string>

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

#define ILL_TEXT(str) L##str

#if UNICODE
	using string = std::wstring;
	using tchar = wchar_t;
#else
	using string = std::string
	using tchar = char;
#endif

using keyCode = u8;
