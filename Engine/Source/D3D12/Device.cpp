#include "Device.hpp"
namespace Illulu::D3D12
{
    void Illulu::D3D12::Device::Initialize(IDXGIFactoryIll* const factory)
    {
    #if defined(DX_DEBUG)
        EnableDebugInterface();
        INFO(L"[DX12] Debug layer enabled");
    #endif
        
        HRESULT hRes{};
        for (u32 i{0}; WIN_OK(factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_dxgiAdapter))); i++)
        {
            hRes = D3D12CreateDevice(m_dxgiAdapter.Get(), D3D12_REQUIRED_FEATURE_LEVEL, IID_PPV_ARGS(&m_d3d12Device));

            if (WIN_OK(hRes))
            {
                auto gpuInfo{GetGPUInfo()};
                auto displayInfos{GetConnectedDisplaysInfo()};

                INFO(L"Graphics Card: {} {:.0f}GB", gpuInfo.gpuName, gpuInfo.dedicatedMemoryInGBs);
                for (const auto& d : displayInfos)
                INFO(L"Monitor: {} {}x{}@{}Hz", d.displayName, d.width, d.height, d.refreshRate);

                m_infoQueue.Initialize(GetDevicePtr());
                
                INFO(L"[DX12] Device created");
                return;
            }
        }

        WIN_CHECK(hRes);
    }
    
    GpuInfo Device::GetGPUInfo()
    {
        DXGI_ADAPTER_DESC3 adapterDesc{};
        m_dxgiAdapter->GetDesc3(&adapterDesc);

        auto gpuName = adapterDesc.Description;
        f32 dedicatedMemoryInGBs{std::ceil((static_cast<f32>(adapterDesc.DedicatedVideoMemory) / (1 << 30)))};

        return GpuInfo
        {
            .gpuName{gpuName},
            .dedicatedMemoryInGBs{dedicatedMemoryInGBs}
        };
    }

    Vector<DisplayInfo> Device::GetConnectedDisplaysInfo()
    {
        HRESULT hRes{};
        u32 i{0};
        ComPtr<IDXGIOutput> outputV0{};

        Vector<DisplayInfo> infos;

        while ((hRes = m_dxgiAdapter->EnumOutputs(i, outputV0.ReleaseAndGetAddressOf())) == S_OK)
        {
            ComPtr<IDXGIOutput6> output{};
            WIN_CHECK(outputV0.As<IDXGIOutput6>(&output));

            DXGI_OUTPUT_DESC1 outputDesc{};
            WIN_CHECK(output->GetDesc1(&outputDesc));

            String monitorName{Util::GetMonitorNameByDXGIName(outputDesc.DeviceName)};

            Vector<DXGI_MODE_DESC1> modeLists{Util::GetDisplayModes(output.Get())};

            DXGI_MODE_DESC1& modeDesc{modeLists.back()};
            f32 nom{static_cast<f32>(modeDesc.RefreshRate.Numerator)};
            f32 denom{static_cast<f32>(modeDesc.RefreshRate.Denominator)};
            f32 refreshRate{nom / denom};
            u32 width{modeDesc.Width};
            u32 height{modeDesc.Height};

            i++;

            infos.emplace_back(
                DisplayInfo
                {
                    .displayName{monitorName},
                    .width{width},
                    .height{height},
                    .refreshRate{static_cast<u32>(refreshRate)}
                }
            );
        }
        return infos;
    }

    void Device::EnableDebugInterface()
    {
        ComPtr<ID3D12DebugIll> debugController;
        WIN_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));

        debugController->EnableDebugLayer();
        debugController->SetEnableGPUBasedValidation(true);
    }
}