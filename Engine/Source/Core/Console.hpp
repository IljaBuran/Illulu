#pragma once

#include "Common.hpp"
#include "WindowsMin.hpp"

#include "Array.hpp"
#include "String.hpp"

#include <utility>
#include <format>

#include <iostream>

namespace Illulu
{
    enum class LogType : u8
    {
        INFO,
        WARNING,
        FATAL,
        COUNT
    };

    FORCEINLINE static constexpr 
    u16 GetTextAttributesByLogType(LogType logType) noexcept
    {
        static constexpr 
        Array<u16, static_cast<u64>(LogType::COUNT)> logTypeAttributes
        {
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
            FOREGROUND_RED | FOREGROUND_INTENSITY
        };

        return logTypeAttributes[static_cast<u64>(logType)];
    }
    
    FORCEINLINE static constexpr 
    StringView GetPrefixByLogType(LogType logType) noexcept
    {
        static constexpr 
        Array<StringView, static_cast<u64>(LogType::COUNT)> logTypePrefixes
        {
            L"[INFO] ",
            L"[WARNING] ",
            L"[FATAL] "
        };

        return logTypePrefixes[static_cast<u64>(logType)];
    }
    
    class Console 
    {
    public:
    
        static void Log(LogType type, StringView str) noexcept
        {
            static Console& console = Console::_Instance();

            String s(GetPrefixByLogType(type));
            s += str;
            s += L"\n";

            SetConsoleTextAttribute(
                console.m_consoleHandle,
                GetTextAttributesByLogType(type)
            );
             
            DWORD written{0};
            WriteConsole(console.m_consoleHandle, s.data(), static_cast<DWORD>(s.size()), &written, nullptr);
        };

        template<typename... Args>
        static void Log(LogType type, std::wformat_string<Args...> format, Args&&... args)
        {
            static Console& console = Console::_Instance();
            
            String s(GetPrefixByLogType(type));
            s += std::format(format, std::forward<Args>(args)...);
            s += L"\n";

            SetConsoleTextAttribute(
                console.m_consoleHandle,
                GetTextAttributesByLogType(type)
            );

            DWORD written{0};
            WriteConsole(console.m_consoleHandle, s.data(), static_cast<DWORD>(s.size()), &written, nullptr);
        }

    private:

        static Console& _Instance() noexcept
        {
            static Console instance;
            return instance;
        }
        
        Console()
        {
            ILL_ASSERT(!m_consoleHandle);

            ILL_VERIFY(AllocConsole());

            ILL_VERIFY(SetConsoleTitle(L"Illulu Logger"));
            m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            ILL_ASSERT(m_consoleHandle != INVALID_HANDLE_VALUE);

            /* Disabling close button on console window */
            HWND hWndConsole = GetConsoleWindow();
            HMENU menu = GetSystemMenu(hWndConsole, false);
            ILL_ASSERT(hWndConsole && menu);
            
            u32 flags = MF_BYCOMMAND | MF_GRAYED | MF_DISABLED;
            EnableMenuItem(menu, SC_CLOSE, flags);

            //const HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
            //ILL_ASSERT(inputHandle != INVALID_HANDLE_VALUE);

            //DWORD mode{};
            //ILL_VERIFY(GetConsoleMode(inputHandle, &mode));

            //mode |= ENABLE_EXTENDED_FLAGS;
            //mode &= ~ENABLE_QUICK_EDIT_MODE;

            //SetConsoleMode(inputHandle, mode);
        }

        ~Console()
        {
            if (m_consoleHandle)
                ILL_VERIFY(FreeConsole());

            m_consoleHandle = nullptr;
        }

        Console(const Console&) = delete;
        Console& operator=(const Console&) = delete;

        Console(Console&&) = delete;
        Console& operator=(Console&&) = delete;

        HANDLE m_consoleHandle;
    };
}
