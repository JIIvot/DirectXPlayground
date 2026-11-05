#include "GameTimer.h"

#include <Windows.h>
#include <utility>

GameTimer::GameTimer()
{
    LARGE_INTEGER countsPerSecond;
    QueryPerformanceFrequency( &countsPerSecond );

    m_secondsPerTick = 1.0f / static_cast<float>( countsPerSecond.QuadPart );
}

void GameTimer::Reset()
{
    LARGE_INTEGER currCounter;
    QueryPerformanceCounter( &currCounter );

    m_baseTicks = currCounter.QuadPart;
    m_currTicks = currCounter.QuadPart;
    m_lastTicks = currCounter.QuadPart;
    m_lastPauseTicks = 0;
    m_isPaused = false;
}

void GameTimer::Start()
{
    if ( !m_isPaused )
    {
        return;
    }

    LARGE_INTEGER currCounter;
    QueryPerformanceCounter( &currCounter );

    m_totalPausedTicks += currCounter.QuadPart - m_lastPauseTicks;
    m_lastTicks = currCounter.QuadPart;
    m_lastPauseTicks = 0;
    m_isPaused = false;
}

void GameTimer::Pause()
{
    if ( m_isPaused )
    {
        return;
    }

    LARGE_INTEGER currCounter;
    QueryPerformanceCounter( &currCounter );

    m_lastPauseTicks = currCounter.QuadPart;
    m_isPaused = true;
}

void GameTimer::Tick()
{
    if ( m_isPaused )
    {
        m_deltaTime = 0.0f;
        return;
    }

    LARGE_INTEGER currCounter;
    QueryPerformanceCounter( &currCounter );

    m_currTicks = currCounter.QuadPart;
    m_deltaTime = std::max( static_cast<float>( m_currTicks - m_lastTicks ) * m_secondsPerTick, 0.0f );
    m_lastTicks = m_currTicks;
}

float GameTimer::GetTotalTime() const
{
    const int64_t currTicks = m_isPaused ? m_lastPauseTicks : m_currTicks;
    return static_cast<float>( currTicks - m_baseTicks - m_totalPausedTicks ) * m_secondsPerTick;
}
