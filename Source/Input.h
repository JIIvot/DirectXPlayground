#pragma once

#include <bitset>

class CInput
{
public:
    void OnKeyDown( size_t key );
    void OnKeyUp( size_t key );

    void OnFrameFinished();

    [[nodiscard]] bool IsKeyPressed( size_t key ) const;

    [[nodiscard]] bool IsKeyJustPressed( size_t key ) const;

private:
    static constexpr size_t kNumKeys = 256;

    std::bitset<kNumKeys> m_currKeys;
    std::bitset<kNumKeys> m_lastKeys;
};
