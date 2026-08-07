#pragma once
#include "Common.hpp"

#include "DX12.hpp"

#include "Vector.hpp"
#include "String.hpp"

namespace Illulu::D3D12::Util
{
    [[nodiscard]]
    Vector<DXGI_MODE_DESC1> GetDisplayModes(IDXGIOutput6* output6);

    void LogAdaptersAndOutputs(IDXGIFactoryIll* factory);

    [[nodiscard]]
    String GetMonitorNameByDXGIName(StringView DXGIName) noexcept;
}