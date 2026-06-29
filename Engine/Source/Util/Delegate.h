#pragma once

#include <vector>

template<typename... Args>
class MulticastDelegate
{
private:

    using Callback = void (*)(void*, Args...);

    struct Listener
    {
        void* object;
        Callback callback;
    };

public:

    template<typename T, void (T::* Method)(Args...)>
    void Add(T* object)
    {
        m_listeners.push_back(
            {object,
            [](void* instance, Args... args)
                {
                    T* typedObj = static_cast<T*>(instance);
                    (typedObj->*Method)(args...);
                }
            }
        );
    }

    void Call(Args... args)
    {
        for (const Listener& listener : m_listeners)
        {
            listener.callback(listener.object, args...);
        }
    }

private:

    std::vector<Listener> m_listeners;
};