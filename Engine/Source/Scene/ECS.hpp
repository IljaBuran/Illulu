#pragma once

#include "Common.hpp"

#include "Vector.hpp"
#include "Queue.hpp"
#include "SparseSet.hpp"

#include "Scene/Component.hpp"

namespace Illulu
{ 
    class ECS
    {
    public:

        using EntityId = u32;

        ECS(u32 maxEntities) noexcept
        {
            for (auto i{ 0u }; i < maxEntities; i++)
            {
                m_freeIds.push(i);
            }
        }



    private:
        
        Queue<EntityId> m_freeIds;
        Vector<ComponentStorage> m_components;
    };
}