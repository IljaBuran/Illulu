#pragma once

#include "Common.hpp"
#include "WindowsMin.hpp"

#include "Array.hpp"
#include "String.hpp"

#include <memory>

#include <utility>
#include <format>

#include "RingBuffer.hpp"
#include "Vector.hpp"

namespace Illulu
{
    enum class LogType : u8
    {
        INFO,
        WARNING,
        FATAL,
        COUNT
    };

    struct LogEntry
    {
        LogType type{ LogType::COUNT };
        String str; // brrr, every entry is allocated
    };

    static constexpr u32 LOG_HISTORY_CAPACITY{ 100 };

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
            L"[INFO]",
            L"[WARNING]",
            L"[FATAL]"
        };

        return logTypePrefixes[static_cast<u64>(logType)];
    }
    
    class Console
    {
    public:
        
        virtual ~Console() = default;
        // here it will receive already formatted string, 
        //  the Console is responsible for is coloring
        virtual void Log(LogType type, StringView str) = 0;
    };

    class WindowsNativeConsole : public Console
    {
    public:
        
        WindowsNativeConsole()
        {
            ILL_ASSERT(!m_consoleHandle);

            ILL_VERIFY(AllocConsole());

            ILL_VERIFY(SetConsoleTitle(L"Illulu Logger"));
            m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            ILL_ASSERT(m_consoleHandle != INVALID_HANDLE_VALUE);

            /* Disabling close button on console window */
            m_hWnd = GetConsoleWindow();
            HMENU menu = GetSystemMenu(m_hWnd, false);
            ILL_ASSERT(m_hWnd && menu);

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

        ~WindowsNativeConsole()
        {
            if (m_consoleHandle)
                ILL_VERIFY(FreeConsole());

            m_consoleHandle = nullptr;
        }
        
        void Log(LogType type, StringView str) override
        {
            SetConsoleTextAttribute(
                m_consoleHandle,
                GetTextAttributesByLogType(type)
            );

            DWORD written{ 0 };
            WriteConsole(m_consoleHandle, str.data(), static_cast<DWORD>(str.size()), &written, nullptr);
        }

    private:

        HANDLE m_consoleHandle{};
        HWND m_hWnd{};
    };

    class ImGUIConsole : public Console
    {
    public:

        ImGUIConsole()
        {
        }

        ~ImGUIConsole()
        {

        }

        void Log(LogType type, StringView str) override
        {
            UNREFERENCED_PARAMETER(type);
            UNREFERENCED_PARAMETER(str);

        }
    };

    class Logger
    {
    public:
    
        static 
        void Log(LogType type, StringView str) noexcept
        {
            String message = std::format(L"{} {}\n", GetPrefixByLogType(type), str);

            m_logHistory.Add(LogEntry{ type, message });

            for (auto& console : m_consoles)
            {
                console->Log(type, message);
            }
        };

        template<typename... Args>
        static 
        void Log(LogType type, std::wformat_string<Args...> format, Args&&... args)
        {
            String s = std::format(
                format,
                std::forward<Args>(args)...
            );

            String message = std::format(L"{} {}\n", GetPrefixByLogType(type), s);

            m_logHistory.Add(LogEntry{ type, message });

            for (auto& console : m_consoles)
            {
                console->Log(type, message);
            }
        }
        
        static void Add(std::unique_ptr<Console> console) noexcept
        {
            ILL_ASSERT(console);

            m_consoles.emplace_back(std::move(console));
        }
        
        [[nodiscard]] static
        const RingBuffer<LogEntry, LOG_HISTORY_CAPACITY>& GetLogHistory()
        {
            return m_logHistory;
        }

    private:

        inline static Vector<std::unique_ptr<Console>> m_consoles;
        inline static RingBuffer<LogEntry, LOG_HISTORY_CAPACITY> m_logHistory;
    };
}
