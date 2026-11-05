#pragma once

#include <cstdint>

class GameTimer
{
public:
    GameTimer();

    void Reset();
    void Start();
    void Pause();
    void Tick();

    [[nodiscard]] float GetDeltaTime() const
    {
        return m_deltaTime;
    }

    [[nodiscard]] float GetTotalTime() const;

private:
    float m_secondsPerTick = 0.0f;
    float m_deltaTime = 0.0f;

    int64_t m_baseTicks = 0;
    int64_t m_totalPausedTicks = 0;
    int64_t m_lastPauseTicks = 0;
    int64_t m_lastTicks = 0;
    int64_t m_currTicks = 0;

    bool m_isPaused = false;
};
