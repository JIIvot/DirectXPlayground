#pragma once

#include <bitset>

class CInput
{
public:
    void OnKeyDown( size_t key )
    {
        m_currKeys[key] = true;
    }

    void OnKeyUp( size_t key )
    {
        m_currKeys[key] = false;
    }

    void Update();

    [[nodiscard]]
    bool IsKeyPressed( size_t key ) const
    {
        return m_currKeys[key];
    }

    [[nodiscard]]
    bool IsKeyJustPressed( size_t key ) const
    {
        return m_currKeys[key] && !m_lastKeys[key];
    }

private:
    static constexpr size_t kNumKeys = 256;

    std::bitset<kNumKeys> m_currKeys;
    std::bitset<kNumKeys> m_lastKeys;
};
