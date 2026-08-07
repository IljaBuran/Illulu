#include "Util.hpp"



namespace Illulu::D3D12::Util
{
    Vector<DXGI_MODE_DESC1> GetDisplayModes(IDXGIOutput6* output6)
    {
        u32 count{};
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

        // get count of modelists
        WIN_CHECK(output6->GetDisplayModeList1(format, 0, &count, nullptr));

        // get the modelists
        Vector<DXGI_MODE_DESC1> modeLists(count);
        WIN_CHECK(output6->GetDisplayModeList1(format, 0, &count, &modeLists[0]));

        return modeLists;
    }

    void LogAdaptersAndOutputs(IDXGIFactoryIll* factory)
    {
        HRESULT hRes;

        u32 i = 0;

        ComPtr<IDXGIAdapter1> adapterV1;

        while ((hRes = factory->EnumAdapters1(i, adapterV1.ReleaseAndGetAddressOf())) != DXGI_ERROR_NOT_FOUND)
        {
            ComPtr<IDXGIAdapterIll> adapter;

            WIN_CHECK(adapterV1.As<IDXGIAdapterIll>(&adapter));

            DXGI_ADAPTER_DESC3 adapterDesc;
            adapter->GetDesc3(&adapterDesc);

            // here it won't compile if UNICODE not used, adapterDesc.Description is hardcoded wchar
            String adapterString = std::format(L"Adapter: {}\n", adapterDesc.Description);
            OutputDebugString(adapterString.c_str());

            u32 j = 0;
            ComPtr<IDXGIOutput> outputV0;
            while ((hRes = adapter->EnumOutputs(j, outputV0.ReleaseAndGetAddressOf())) == S_OK)
            {
                ComPtr<IDXGIOutput6> output;
                WIN_CHECK(outputV0.As<IDXGIOutput6>(&output));

                DXGI_OUTPUT_DESC1 outputDesc;
                WIN_CHECK(output->GetDesc1(&outputDesc));

                // here it won't compile if UNICODE not used, outputDesc.DeviceName is hardcoded wchar
                String outputString = std::format(L"\tMonitor: {} \n", outputDesc.DeviceName);
                OutputDebugString(outputString.c_str());

                Vector<DXGI_MODE_DESC1> modeLists = GetDisplayModes(output.Get());

                for (const auto& x : modeLists)
                {
                    f32 nom = static_cast<f32>(x.RefreshRate.Numerator);
                    f32 denom = static_cast<f32>(x.RefreshRate.Denominator);
                    f32 refreshRate = nom / denom;

                    u32 width = x.Width;
                    u32 height = x.Height;

                    OutputDebugString(std::format(L"\t\t{}x{}@{}Hz\n", width, height, refreshRate).c_str());
                }
                j++;
            }
            assert(hRes == DXGI_ERROR_NOT_FOUND);

            i++;
        }

        assert(hRes == DXGI_ERROR_NOT_FOUND);

    }
    
    [[nodiscard]]
    String GetMonitorNameByDXGIName(StringView DXGIName) noexcept
    {
        u32 pathCount{};
        u32 modeCount{};

        i32 result = GetDisplayConfigBufferSizes(
            QDC_ONLY_ACTIVE_PATHS,
            &pathCount,
            &modeCount
        );

        if (result != ERROR_SUCCESS)
            return L"Unknown";

        Vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
        Vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

        result = QueryDisplayConfig(
            QDC_ONLY_ACTIVE_PATHS,
            &pathCount,
            paths.data(),
            &modeCount,
            modes.data(),
            nullptr
        );

        if (result != ERROR_SUCCESS)
            return L"Unknown";

        paths.resize(pathCount);

        for (const DISPLAYCONFIG_PATH_INFO& path : paths)
        {
            DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName
            {
                .header
                {
                    .type{DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME},
                    .size{sizeof(sourceName)},
                    .adapterId{path.sourceInfo.adapterId},
                    .id{path.sourceInfo.id}
                }
            };

            if (DisplayConfigGetDeviceInfo(&sourceName.header) != ERROR_SUCCESS)
                continue;

            if (DXGIName != sourceName.viewGdiDeviceName)
                continue;

            DISPLAYCONFIG_TARGET_DEVICE_NAME targetName
            {
                .header
                {
                    .type{DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME},
                    .size{sizeof(targetName)},
                    .adapterId{path.targetInfo.adapterId},
                    .id{path.targetInfo.id}
                }
            };

            if (DisplayConfigGetDeviceInfo(&targetName.header) != ERROR_SUCCESS)
                return L"Unknown";

            if (targetName.monitorFriendlyDeviceName[0] == L'\0')
                return L"Unknown";

            return targetName.monitorFriendlyDeviceName;
        }

        return L"Unknown";
    }
}

