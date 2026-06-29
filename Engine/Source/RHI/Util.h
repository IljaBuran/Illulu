#pragma once
#include "Common.h"

#include "DX12.h"

#include <vector>

namespace Illulu
{
    std::vector<DXGI_MODE_DESC1> GetDisplayModes(IDXGIOutput6* output6);
    void LogAdaptersAndOutputs(IDXGIFactoryIll* factory);
    void EnableDebugLayer();
}