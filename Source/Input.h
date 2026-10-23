#pragma once

#include <vector>
#include <SDL3/SDL_scancode.h>

class CInput
{
public:
    void Init();
    void Update();

    [[nodiscard]]
    bool IsKeyPressed( SDL_Scancode code ) const
    {
        return m_pCurrKeys[code];
    }

    [[nodiscard]]
    bool IsKeyJustPressed( SDL_Scancode code ) const
    {
        return m_pCurrKeys[code] && !m_lastKeys[code];
    }

private:
    const bool* m_pCurrKeys = nullptr;
    std::vector<uint8_t> m_lastKeys;
    int32_t m_numKeys = 0;
};
