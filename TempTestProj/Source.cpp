#include <Source/Common/Types.hpp>

#include "Windows.h"

#include <print>
#include <string>
#include <iostream>

#include <memory>

using namespace Illulu;

constexpr DWORD BUFFER_SIZE{1024 * 64};

i32 main()
{
    byte* buffer = new byte[BUFFER_SIZE];
    OVERLAPPED overlapped{};

    HANDLE event{};
    HANDLE dirHandle = CreateFileW(
        L"test",
        FILE_LIST_DIRECTORY,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        nullptr
    );

    if (dirHandle == INVALID_HANDLE_VALUE)
    {
        std::println("dirHandle == INVALID_HANDLE_VALUE");
        return 1;
    }

    event = CreateEventW(
        nullptr,
        true, // manual reset turned on
        false,
        nullptr
    );

    if (!event)
    {
        std::println("event is null");
        DWORD err = GetLastError();
        std::println("Error: {}", err);
        return 1;
    }

    overlapped.hEvent = event;

    bool res = ReadDirectoryChangesW(
        dirHandle,
        buffer,
        BUFFER_SIZE,
        false,
        FILE_NOTIFY_CHANGE_LAST_WRITE,
        nullptr,
        &overlapped,
        nullptr
    );

    if (!res)
    {
        std::println("ReadDirectoryChangesW failed");
        DWORD err = GetLastError();
        std::print("Error: {}", err);
        return 1;
    }

    while (true)
    {
        if (WaitForSingleObject(event, 0) == WAIT_OBJECT_0)
        {
            SYSTEMTIME st{};

            GetLocalTime(&st);

            std::println("{}-{}-{} {}:{}:{}.{}",
                st.wDay, st.wMonth, st.wYear,
                st.wHour, st.wMinute, st.wMilliseconds, st.wMilliseconds);

            DWORD bytesTransferred{};


            if (GetOverlappedResult(
                dirHandle,
                &overlapped,
                &bytesTransferred,
                false))
            {
                std::println("Bytes transfered: {}", bytesTransferred);
                auto* info{reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer)};
                while (true)
                {
                    fiole
                    if (wcscmp(m_fileName.data(), )
                    
                    if (info->Action == FILE_ACTION_MODIFIED)
                    {
                        std::wstring name(info->FileName, info->FileNameLength / sizeof(wchar_t));
                        std::wcout << L"File changed: " << name << '\n';
                    }
                    if (info->NextEntryOffset == 0)
                        break;

                    info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                        reinterpret_cast<byte*>(info) + info->NextEntryOffset);
                }
            }

            ResetEvent(event);

            overlapped = {};
            overlapped.hEvent = event;

            res = ReadDirectoryChangesW(
                dirHandle,
                buffer,
                static_cast<DWORD>(BUFFER_SIZE),
                false,
                FILE_NOTIFY_CHANGE_LAST_WRITE,
                nullptr,
                &overlapped,
                nullptr
            );

            if (!res)
            {
                std::println("ReadDirectoryChangesW failed");
                DWORD err = GetLastError();
                std::println("Error: {}", err);
                return 1;
            }
        }
    }

    std::println("SUCCESS");

    CloseHandle(event);
    CloseHandle(dirHandle);

    delete[] buffer;

    return 0;
}