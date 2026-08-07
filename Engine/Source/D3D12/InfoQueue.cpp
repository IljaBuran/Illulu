#include "InfoQueue.hpp"

namespace Illulu::D3D12
{
    void InfoQueue::Initialize(ID3D12DeviceIll* const device)
    {
        ILL_ASSERT(device);

        WIN_CHECK(device->QueryInterface(IID_PPV_ARGS(&m_infoQueue)));
        WIN_CHECK(m_infoQueue->RegisterMessageCallback(
            _callback_InfoQueueFunc,
            D3D12_MESSAGE_CALLBACK_FLAG_NONE,
            nullptr, &m_infoQueueCallbackCookie));

        ILL_ASSERT(m_infoQueueCallbackCookie);

        INFO(L"[DX12] Debug InfoQueue created and message callback registered");
    }

    void InfoQueue::Destroy()
    {
        if (m_infoQueue)
        {
            m_infoQueue->UnregisterMessageCallback(m_infoQueueCallbackCookie);
            m_infoQueueCallbackCookie = 0;
        }
    }

    String InfoQueue::GetCategoryString(D3D12_MESSAGE_CATEGORY category)
    {
        constexpr std::string_view prefix = "D3D12_MESSAGE_CATEGORY_";

        std::string_view name = magic_enum::enum_name(category);

        if (name.empty())
            return L"Unknown";

        if (name.starts_with(prefix))
            name.remove_prefix(prefix.size());

        return String{name.begin(), name.end()};
    }

    String InfoQueue::GetSeverityString(D3D12_MESSAGE_SEVERITY severity)
    {
        constexpr std::string_view prefix = "D3D12_MESSAGE_SEVERITY_";

        std::string_view name = magic_enum::enum_name(severity);

        if (name.empty())
            return L"Unknown";

        if (name.starts_with(prefix))
            name.remove_prefix(prefix.size());

        return String{name.begin(), name.end()};
    }

    String InfoQueue::GetIdString(D3D12_MESSAGE_ID messageId)
    {
        constexpr std::string_view prefix = "D3D12_MESSAGE_ID_";

        std::string_view name = magic_enum::enum_name(messageId);

        if (name.empty())
            return L"Unknown";

        if (name.starts_with(prefix))
            name.remove_prefix(prefix.size());

        return String{name.begin(), name.end()};
    }

    void InfoQueue::_callback_InfoQueueFunc(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID id, LPCSTR pDescription, UNUSED void* pContext)
    {
        LogType logType{};
        switch (severity)
        {
            case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                [[fallthrough]];
            case D3D12_MESSAGE_SEVERITY_ERROR:
            {
                logType = LogType::FATAL;
                break;
            }
            case D3D12_MESSAGE_SEVERITY_WARNING:
            {
                logType = LogType::WARNING;
                break;
            }
            case D3D12_MESSAGE_SEVERITY_MESSAGE:
                [[fallthrough]];
            case D3D12_MESSAGE_SEVERITY_INFO:
            {
                logType = LogType::INFO;
                break;
            }
        }
        std::string strA(pDescription);
        String strW = Utf8ToWide(strA);
        
        String str = std::format(
            L"Category: {}\n        ID: {}\n        Description: {}", 
            InfoQueue::GetCategoryString(category),
            InfoQueue::GetIdString(id),
            strW
        );

        Console::Log(logType, str);
    }
}