#pragma once

#include "Types.hpp"
#include "WindowsMin.hpp"
#include "String.hpp"

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

        String GetErrorMessage() const
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

#define WIN_OK(HRES_EXPR) SUCCEEDED(HRES_EXPR)
#define WIN_CHECK(x)                                                          \
{												                              \
    HRESULT __hRes = x;							                              \
    if (FAILED(__hRes))							                              \
    {										                                  \
        __debugbreak();                                                       \
        String __fileNameW(__FILEW__);		                                  \
        throw ILLException(__hRes, String(L#x), String(__FILEW__), __LINE__); \
    }                                                                         \
}
#if defined(_DEBUG)

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
    //#define WIN_CHECK(x) x
    #define ILL_ASSERT(expr) ((void)0)
    #define ILL_VERIFY(expr) ((void)(expr))
#endif
}