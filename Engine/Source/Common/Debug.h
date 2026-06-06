#pragma once

#include "Types.h"
#include "WindowsMin.h"

#include <type_traits>
#include <stdexcept>
#include <exception>
#include <iterator>
#include <bitset>
#include <format>
#include <comdef.h>

namespace Illulu
{
	class ILLException
	{
	public:

		ILLException() = delete;
		explicit ILLException(HRESULT hRes, const String& functionName, const String& fileName, i32 lineNumber)
			: m_hRes(hRes), m_functionName(functionName), m_fileName(fileName), m_lineNumber(lineNumber) {}

		String GetMessage() const
		{
			_com_error err(m_hRes);
			String errMsg = err.ErrorMessage();
			
			return std::format(L"File: {}\nLine: {}\nFunction: {}, HRESULT error code: {}, HRESULT error msg: {}",
				m_fileName, m_lineNumber, m_functionName, static_cast<i32>(m_hRes), errMsg);
		}

	private:
		
		HRESULT m_hRes;
		String  m_functionName;
		String  m_fileName;
		i32     m_lineNumber;
	};

	template<typename T>
	concept Container = requires(T container)
	{
		std::begin(container);
		std::end(container);
	};

	template<Container T>
	inline String PrintContainer(const T& container) noexcept
	{
		String str = L"[";
		bool first = true;

		for (const auto& value : container)
		{
			if (!first)
				str += L", ";

			str += std::format(L"{}", value);
			first = false;
		}

		str += L"]\n";
		return str;
	}

	template<u64 size>
	inline String BitsetToKeys(const std::bitset<size>& bs) noexcept
	{
		String str = L"[";
		bool first = true;

		for (u64 i = 0; i < size; i++)
		{
			if (bs.test(i) != true)
				continue;
			
			if (!first)
				str += L", ";

			str += std::format(L"{}", static_cast<char>(i));
			first = false;
		}

		str += L"]\n";
		return str;
	}

#if defined(_DEBUG)
	#define WIN_CHECK(x)                                                          \
	{												                              \
		HRESULT __hRes = x;							                              \
		if (FAILED(__hRes))							                              \
		{										                                  \
			String __fileNameW(__FILEW__);		                                  \
			throw ILLException(__hRes, String(L#x), String(__FILEW__), __LINE__); \
		}                                                                         \
	}

	#define ILL_ASSERT(expr)                                      \
		do                                                        \
		{                                                         \
			if (!(expr))                                          \
			{                                                     \
				__debugbreak();                                   \
			}                                                     \
		} while (false)

	#define ILL_VERIFY(expr) ILL_ASSERT(expr)
#else
	#define WIN_CHECK(x) x
	#define ILL_ASSERT(expr) ((void)0)
	#define ILL_VERIFY(expr) ((void)(expr))
#endif
}

