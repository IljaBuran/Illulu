#pragma once

#include "Common.hpp"
#include "String.hpp"
#include "Util/Delegate.hpp"

#include "chrono"

class FileWatcher
{
public:

    void Intialize(StringView directoryPath, StringView fileName, MulticastDelegate<>&& delegate);

    void OnUpdate();

    ~FileWatcher();

private:

    static constexpr DWORD BUFFER_SIZE{ 1024 * 64 };

    String m_directoryPath{};
    String m_fileName{};

    byte*      m_pBuffer{};
    HANDLE     m_hEvent{};
    HANDLE     m_hDirectory{};
    OVERLAPPED m_overlapped{};

    std::chrono::steady_clock::time_point m_lastChange;

    MulticastDelegate<> m_onChangeDelegate;
};