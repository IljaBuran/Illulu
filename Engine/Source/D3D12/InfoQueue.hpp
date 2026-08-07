#pragma once

#include "Common.hpp"

#include "String.hpp"

#include "DX12.hpp"

#include "magic_enum.hpp"

// overriding search range
template<>
struct magic_enum::customize::enum_range<D3D12_MESSAGE_ID>
{
    static constexpr int min =
        static_cast<int>(D3D12_MESSAGE_ID_UNKNOWN);

    static constexpr int max =
        static_cast<int>(D3D12_MESSAGE_ID_D3D12_MESSAGES_END);
};

namespace Illulu::D3D12
{
    class InfoQueue
    {
    public:

        InfoQueue() = default;

        void Initialize(ID3D12DeviceIll* const device);
        void Destroy();

        static String GetCategoryString(D3D12_MESSAGE_CATEGORY category);
        static String GetSeverityString(D3D12_MESSAGE_SEVERITY severity);
        static String GetIdString(D3D12_MESSAGE_ID messageId);

        InfoQueue(const InfoQueue&) = delete;
        InfoQueue(InfoQueue&&) = delete;
        void operator=(const InfoQueue&) = delete;
        void operator=(InfoQueue&&) = delete;

    private:

        static void _callback_InfoQueueFunc(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID id, LPCSTR pDescription, UNUSED void* pContext);

    private:
        ComPtr<ID3D12InfoQueueIll> m_infoQueue;
        DWORD                      m_infoQueueCallbackCookie;
    };
}


