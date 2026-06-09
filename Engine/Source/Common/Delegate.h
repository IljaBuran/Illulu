#pragma once

#include "Common.h"

namespace Illulu
{
    class DelegateHandle
    {
    public:

        constexpr DelegateHandle() noexcept = default;

        [[nodiscard]] static DelegateHandle Create() noexcept
        {
            DelegateHandle handle;
            handle.m_id = m_nextId++;
            return handle;
        }
        
        [[nodiscard]] bool IsValid() const noexcept
        {
            return m_id != INVALID_ID;
        }

        void Reset() noexcept
        {
            m_id = INVALID_ID;
        }

        friend bool operator==(DelegateHandle lhs, DelegateHandle rhs) noexcept
        {
            return lhs.m_id == rhs.m_id;
        }

        friend bool operator!=(DelegateHandle lhs, DelegateHandle rhs) noexcept
        {
            return !(lhs == rhs);
        }

    private:
    
        constexpr static u64 INVALID_ID = 0;
    
    private:
        
        u64 m_id = INVALID_ID;
        inline static u64 m_nextId = 1;
    };

    template<typename... Args>
    class RawDelegate
    {
        using Stub = void(*)(void* object, Args... args);

    public:

        constexpr RawDelegate() noexcept = default;

        [[nodiscard]] bool IsBound() const noexcept
        {
            return m_stub != nullptr;
        }

        [[nodiscard]] bool IsBoundTo(const void* object) const noexcept
        {
            ILL_ASSERT(object);
            return object != nullptr && m_object == object;
        }

        void Clear() noexcept
        {
            m_object = nullptr;
            m_stub = nullptr;
        }

        void Execute(Args... args) const
        {
            ILL_ASSERT(IsBound());
            m_stub(m_object, std::forward<Args>(args)...);
        }

        template<typename T, void (T::*Method)(Args...)>
        static RawDelegate Create(T* object) noexcept
        {
            ILL_ASSERT(object);

            RawDelegate delegate;
            delegate.m_object = object;
            delegate.m_stub = [](void* object, Args... args)
            {
                T* typedObject = static_cast<T*>(object);
                (typedObject->*Method)(std::forward<Args>(args)...);
            };

            return delegate;
        }

        template<typename T, void (T::*Method)(Args...) const>
        static RawDelegate Create(const T* object) noexcept
        {
            ILL_ASSERT(object);

            RawDelegate delegate;
            delegate.m_object = object;
            delegate.m_stub = [](void* object, Args... args)
            {
                const T* typedObject = static_cast<const T*>(object);
                (typedObject->*Method)(std::forward<Args>(args)...);
            };

            return delegate;
        }

        template<typename T, void (*Function)(Args...)>
        static RawDelegate CreateStatic() noexcept
        {
            RawDelegate delegate;
            delegate.m_object = nullptr;
            delegate.m_stub = [](void*, Args... args)
            {
                (Function)(std::forward<Args>(args)...);
            };

            return delegate;
        }

    private:

        void* m_object;
        Stub m_stub;
    };
}
