#pragma once

#include "Common.h"

#include "DX12.h"

namespace Illulu
{
    class InfoQueue
    {
    public:

        void CreateAndSetCallback(ID3D12DeviceIll* device);
        void Destroy();

    private:

        static void _callback_InfoQueueFunc(
            D3D12_MESSAGE_CATEGORY Category,
            D3D12_MESSAGE_SEVERITY Severity,
            D3D12_MESSAGE_ID ID,
            LPCSTR pDescription,
            void* pContext);
        

    private:
        ComPtr<ID3D12InfoQueueIll> m_infoQueue;
        DWORD                      m_infoQueueCallbackCookie;
    };

}


