#pragma once

#include "Common.hpp"

#include "DX12.hpp"

#include "String.hpp"
#include "Vector.hpp"
#include "HashMap.hpp"

namespace Illulu
{
    using MaterialHandle = u32;

    using namespace DirectX;

    struct PBRMaterial
    {
        XMFLOAT4 albedo;
        XMFLOAT3 emissive;
        f32      roughness;
        f32      metallic;
        f32      reflectance;
        XMFLOAT2 textureScale;
    };

    struct Primitive
    {
        Vector<Vertex> vertices;
        Vector<u32> indices;

        MaterialHandle materialHandle;
    };

    struct Mesh
    {
        Vector<Primitive> primitives;
    };

    // Imports CPU mesh definitions in glTF coordinates; returns empty on failure.
    NODISCARD Vector<Mesh> LoadModel(NarrowString path);

    extern Vector<PBRMaterial> g_materialStorage;
}
