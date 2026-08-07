#pragma once

#include "Common.hpp"

namespace Illulu
{
    class Timer
    {
    public:

        Timer() noexcept;
        
        [[nodiscard]] f32 GetTotalTime() const noexcept;
        [[nodiscard]] f32 GetDelta() const noexcept;

        void Reset() noexcept;
        void Start() noexcept;
        void Stop() noexcept;
        void Tick() noexcept;

    private:

        f64 m_secondsPerCount = 0.0;
        f64 m_deltaTime       = 0.0;

        i64 m_baseTime   = 0;
        i64 m_pausedTime = 0;
        i64 m_stopTime   = 0;
        i64 m_prevTime   = 0;
        i64 m_currTime   = 0;

        bool m_stopped = false;
    };
}