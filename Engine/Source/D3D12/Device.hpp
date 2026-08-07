#include "Common.hpp"

#include "Vector.hpp"

#include "DX12.hpp"

#include "D3D12/Util.hpp"
#include "D3D12/HardwareInfo.hpp"
#include "D3D12/InfoQueue.hpp"

namespace Illulu::D3D12
{
    class Device
    {
    public:
        
        Device() = default;
        
        void Initialize(IDXGIFactoryIll* const factory);

        ID3D12DeviceIll* const GetDevicePtr()
        {
            ILL_ASSERT(m_d3d12Device);
            return m_d3d12Device.Get();
        }

        IDXGIAdapterIll* const GetAdapterPtr()
        {
            ILL_ASSERT(m_dxgiAdapter);
            return m_dxgiAdapter.Get();
        }

        ID3D12DeviceIll* const operator->()
        {
            return GetDevicePtr();
        }

        operator ID3D12DeviceIll* const()
        {
            return m_d3d12Device.Get();
        }

        GpuInfo GetGPUInfo();
        Vector<DisplayInfo> GetConnectedDisplaysInfo();

        Device(const Device&) = delete;
        Device(Device&&) = delete;
        void operator=(const Device&) = delete;
        void operator=(Device&&) = delete;

    private:

        ComPtr<ID3D12DeviceIll> m_d3d12Device;
        ComPtr<IDXGIAdapterIll> m_dxgiAdapter;
        InfoQueue               m_infoQueue;

    private:

        void EnableDebugInterface();
    };
}