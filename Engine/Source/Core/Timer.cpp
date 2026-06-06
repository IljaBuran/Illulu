#include "Timer.h"

#include "WindowsMin.h"

namespace Illulu
{
    f32 Timer::GetTotalTime() const noexcept
    {
        return m_stopped ? static_cast<f32>(((m_stopTime - m_pausedTime) - m_baseTime) * m_secondsPerCount)
                         : static_cast<f32>(((m_currTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
    }

    f32 Timer::GetDelta() const noexcept
    {
        return f32();
    }

    void Timer::Reset() noexcept
    {
        i64 currTime;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

        m_baseTime = currTime;
        m_prevTime = currTime;
        m_stopTime = 0;
        m_stopped = false;
    }

    void Timer::Start() noexcept
    {
        i64 startTime;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&startTime));

        if (m_stopped)
        {
            m_pausedTime += (startTime - m_stopTime);

            m_prevTime = startTime;
        
            m_stopTime = 0;
            m_stopped = false;
        }
    }

    void Timer::Stop() noexcept
    {
        if (!m_stopped)
        {
            i64 currTime;
            QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

            m_stopTime = true;
            m_stopped = true;
        }
    }

    void Timer::Tick() noexcept
    {
        if (m_stopped) [[unlikely]]
        {
            m_deltaTime = 0.0;
            return;
        }

        i64 currTime;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));
        m_currTime = currTime;

        m_deltaTime = (m_currTime - m_prevTime) * m_secondsPerCount;
        m_prevTime = m_currTime;

        // according to msdn there are scenarios it can be negative...
        m_deltaTime = (m_deltaTime >= 0.0f) ? m_deltaTime : 0.0f;
    }
}

