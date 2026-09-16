#pragma once

#include "Common.hpp"

#include "DX12.hpp"

#include "Vector.hpp"
#include "SparseSet.hpp"

#include "HashMap.hpp"

#include "Data.hpp"

#include "GraphicsMemory.h" // ThirdParty/directxtk12/Inc/GraphicsMemory.h


namespace Illulu
{
    enum class RenderLayer : u8
    {
        Opaque,
        Transparent,
        Debug,
        Sky,
        Count
    };

    struct RenderItem
    {
        DirectX::XMFLOAT4X4 worldMat;
        ObjectConstants objectConstants;
        DirectX::GraphicsResource memHandleToObjectCB;

        D3D12_PRIMITIVE_TOPOLOGY primitiveType{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };

        u32 indexCount{};
        u32 startIndexLocation{};
        i32 baseVertexLocation{};
    };

    //using EntityId = u32;
    //class Scene;

    //namespace SceneLoader
    //{
    //    Scene LoadScene()
    //    {
    //        // TODO:
    //        return Scene{};
    //    }

    //    void SaveScene()
    //    {
    //    }
    //}

    //class Scene
    //{
    //public:

    //    void Update();


    //private:

    //    friend Scene SceneLoader::LoadScene();
    //    friend void  SceneLoader::SaveScene();
    //};
}