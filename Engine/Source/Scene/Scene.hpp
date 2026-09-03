#pragma once

#include "Common.hpp"

#include "SparseSet.hpp"

namespace Illulu
{
    using EntityId = u32;

    class Scene;

    namespace SceneLoader
    {
        Scene LoadScene()
        {
            // TODO:
            return Scene{};
        }

        void SaveScene()
        {
        }
    }

    class Scene
    {
    public:

        void Update();


    private:

        i32 i;

        friend Scene SceneLoader::LoadScene();
        friend void  SceneLoader::SaveScene();
    };
}