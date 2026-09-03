#pragma once

#if defined(ILL_STL)
#include <string>
#include <format>

using String = std::wstring;
using StringView = std::wstring_view;

using NarrowString = std::string;

#endif

#include <WindowsMin.hpp>
inline String Utf8ToWide(const NarrowString& str, unsigned int codePage = CP_UTF8)
{
    if (str.empty())
    {
        return String{};
    }

    const int required = MultiByteToWideChar(
        codePage,
        0,
        str.data(),
        static_cast<int>(str.size()),
        nullptr,
        0
    );

    if (required == 0)
    {
        return String{};
    }

    String result;
    result.resize(required);

    const int converted = MultiByteToWideChar(
        codePage,
        0,
        str.data(),
        static_cast<int>(str.size()),
        result.data(),
        required
    );

    if (converted == 0)
    {
        return String{};
    }

    return result;
}

inline NarrowString WideToUtf8(const String& str, unsigned int codePage = CP_UTF8)
{
    if (str.empty())
    {
        return NarrowString{};
    }

    const int required = WideCharToMultiByte(
        codePage,
        0,
        str.data(),
        static_cast<int>(str.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (required == 0)
    {
        return NarrowString{};
    }

    NarrowString result;
    result.resize(required);

    const int converted = WideCharToMultiByte(
        codePage,
        0,
        str.data(),
        static_cast<int>(str.size()),
        result.data(),
        required,
        nullptr,
        nullptr
    );

    if (converted == 0)
    {
        return NarrowString{};
    }

    return result;
}