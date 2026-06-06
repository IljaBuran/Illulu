#pragma once

#include "Common.h"

namespace Illulu
{
    class Timer
    {
        f32 GetTotalTime() const noexcept;
        f32 GetDelta() const noexcept;

        void Reset() noexcept;
        void Start() noexcept;
        void Stop() noexcept;
        void Tick() noexcept;

    private:

        f64 m_secondsPerCount =  0.0f;
        f64 m_deltaTime       = -1.0f;

        i64 m_baseTime   = 0;
        i64 m_pausedTime = 0;
        i64 m_stopTime   = 0;
        i64 m_prevTime   = 0;
        i64 m_currTime   = 0;

        bool m_stopped = false;
    };
}