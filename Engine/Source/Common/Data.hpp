#pragma once

#include "Array.hpp"

#include "Common.hpp"
#include "DX12.hpp"

using namespace Illulu;
using namespace DirectX;

extern Array<ColorVertex, 8> boxVertices;

extern Array<u16, 36> boxIndices;

struct ObjectConstants
{
    DirectX::XMFLOAT4X4 M{};
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 VP{};
};