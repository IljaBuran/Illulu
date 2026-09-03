#pragma once

#include "Common.hpp"

#include <concepts>

struct ComponentStorage
{
    void* m_pComponentSparseSet;
    void (*destructor)(void*);
};

template<typename T>
concept IsComponent = requires(T t)
{
    t.OnInit();
    t.OnUpdate();
    t.OnDestroy();
};