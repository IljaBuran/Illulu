#include "Timer.h"

#include "WindowsMin.h"

namespace Illulu
{
    Timer::Timer() noexcept
    {
        i64 countsPerSecond = 0;
        QueryPerformanceFrequency(
            reinterpret_cast<LARGE_INTEGER*>(&countsPerSecond)
        );

        m_secondsPerCount = 1.0 / static_cast<f64>(countsPerSecond);

        Reset();
    }

    f32 Timer::GetTotalTime() const noexcept
    {
        const i64 endTime = m_stopped ? m_stopTime : m_currTime;

        return static_cast<f32>(
            ((endTime - m_pausedTime) - m_baseTime) *
            m_secondsPerCount
        );
    }

    f32 Timer::GetDelta() const noexcept
    {
        return static_cast<f32>(m_deltaTime);
    }

    void Timer::Reset() noexcept
    {
        i64 currTime = 0;
        QueryPerformanceCounter(
            reinterpret_cast<LARGE_INTEGER*>(&currTime)
        );

        m_baseTime = currTime;
        m_prevTime = currTime;
        m_currTime = currTime;
        m_stopTime = 0;
        m_pausedTime = 0;
        m_deltaTime = 0.0;
        m_stopped = false;
    }

    void Timer::Start() noexcept
    {
        if (!m_stopped)
            return;

        i64 startTime = 0;
        QueryPerformanceCounter(
            reinterpret_cast<LARGE_INTEGER*>(&startTime)
        );

        m_pausedTime += startTime - m_stopTime;
        m_prevTime = startTime;
        m_stopTime = 0;
        m_stopped = false;
    }

    void Timer::Stop() noexcept
    {
        if (m_stopped)
            return;

        QueryPerformanceCounter(
            reinterpret_cast<LARGE_INTEGER*>(&m_stopTime)
        );

        m_stopped = true;
    }

    void Timer::Tick() noexcept
    {
        if (m_stopped)
        {
            m_deltaTime = 0.0;
            return;
        }

        QueryPerformanceCounter(
            reinterpret_cast<LARGE_INTEGER*>(&m_currTime)
        );

        m_deltaTime =
            static_cast<f64>(m_currTime - m_prevTime) *
            m_secondsPerCount;

        m_prevTime = m_currTime;

        if (m_deltaTime < 0.0)
            m_deltaTime = 0.0;
    }
}