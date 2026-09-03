#include "FileWatcher.hpp"

void FileWatcher::Intialize(StringView directoryPath, StringView fileName, MulticastDelegate<>&& delegate)
{
    using namespace Illulu;
    
    m_directoryPath = directoryPath;
    m_fileName = fileName;
    m_pBuffer = new byte[BUFFER_SIZE];
    m_onChangeDelegate = std::move(delegate);

    m_hDirectory = CreateFileW(
        directoryPath.data(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        nullptr
    );

    if (m_hDirectory == INVALID_HANDLE_VALUE)
    {
        FATAL(L"FileWatcher: couldn't create file handle");
    }

    m_hEvent = CreateEventW(
        nullptr,
        true, // manual reset turned on
        false,
        nullptr
    );

    if (!m_hEvent)
    {
        DWORD err = GetLastError();
        FATAL(L"FileWatcher: couldn't create event, Error: {}", err);
    }

    m_overlapped.hEvent = m_hEvent;

    bool res = ReadDirectoryChangesW(
        m_hDirectory,
        m_pBuffer,
        BUFFER_SIZE,
        false,
        FILE_NOTIFY_CHANGE_LAST_WRITE,
        nullptr,
        &m_overlapped,
        nullptr
    );

    if (!res)
    {
        DWORD err = GetLastError();
        FATAL(L"FileWatcher: ReadDirectoryChangesW failed, Error: {}", err);
    }
}

void FileWatcher::OnUpdate()
{
    using namespace Illulu;
    

    if (WaitForSingleObject(m_hEvent, 0) == WAIT_OBJECT_0)
    {

        DWORD bytesTransferred{};

        if (GetOverlappedResult(
            m_hDirectory,
            &m_overlapped,
            &bytesTransferred,
            false))
        {
            auto* info{reinterpret_cast<FILE_NOTIFY_INFORMATION*>(m_pBuffer)};
            while (true)
            {
                auto now{ std::chrono::steady_clock::now() };
                if (info->Action == FILE_ACTION_MODIFIED)
                {
                    StringView currFileName(info->FileName, info->FileNameLength / sizeof(wchar));
                    
                    if (currFileName == m_fileName)
                    {
                        if (now - m_lastChange > std::chrono::milliseconds(200))
                        {
                            m_onChangeDelegate.Call();

                            m_lastChange = now;
                        }
                    }
                }
                if (info->NextEntryOffset == 0)
                    break;

                info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                    reinterpret_cast<byte*>(info) + info->NextEntryOffset);
            }
        }

        ResetEvent(m_hEvent);

        m_overlapped = {};
        m_overlapped.hEvent = m_hEvent;

        bool res = ReadDirectoryChangesW(
            m_hDirectory,
            m_pBuffer,
            static_cast<DWORD>(BUFFER_SIZE),
            false,
            FILE_NOTIFY_CHANGE_LAST_WRITE,
            nullptr,
            &m_overlapped,
            nullptr
        );

        if (!res)
        {
            DWORD err = GetLastError();
            FATAL(L"FileWatcher: ReadDirectoryChangesW failed, Error: {}", err);
        }
    }
}

FileWatcher::~FileWatcher()
{
    if (m_hDirectory != INVALID_HANDLE_VALUE)
    {
        CancelIoEx(m_hDirectory, &m_overlapped);
        WaitForSingleObject(m_hEvent, INFINITE);
        CloseHandle(m_hDirectory);
    }

    if (m_hEvent)
    {
        CloseHandle(m_hEvent);
    }
    delete[] m_pBuffer;
}
