#include "InfoQueue.h"

namespace Illulu
{
    void InfoQueue::CreateAndSetCallback(ID3D12DeviceIll* device)
    {
        ILL_ASSERT(device);

        WIN_CHECK(device->QueryInterface(IID_PPV_ARGS(&m_infoQueue)));
        WIN_CHECK(m_infoQueue->RegisterMessageCallback(
            _callback_InfoQueueFunc,
            D3D12_MESSAGE_CALLBACK_FLAG_NONE,
            nullptr, &m_infoQueueCallbackCookie));

        ILL_ASSERT(m_infoQueueCallbackCookie);
    }

    void InfoQueue::Destroy()
    {
        if (m_infoQueue)
        {
            m_infoQueue->UnregisterMessageCallback(m_infoQueueCallbackCookie);
            m_infoQueueCallbackCookie = 0;
        }
    }

    void InfoQueue::_callback_InfoQueueFunc(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID id, LPCSTR pDescription, void* pContext)
    {
        //TODO
        (void)category;
        (void)severity;
        (void)id;
        (void)pDescription;
        (void)pContext;

        //MessageBox(nullptr, L"I got called from callback", L"InfoQueue", MB_OK);
    }
}