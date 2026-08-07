#include "Common.hpp"

namespace Illulu::D3D12
{
    struct DisplayInfo
    {
        String displayName{};
        u32 width{};
        u32 height{};
        u32 refreshRate{};
    };

    struct GpuInfo
    {
        String gpuName{};
        f32 dedicatedMemoryInGBs{};
    };
}