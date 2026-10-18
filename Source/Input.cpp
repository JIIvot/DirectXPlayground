#include "Input.h"

#include <SDL3/SDL_keyboard.h>

void CInput::Init()
{
    m_pCurrKeys = SDL_GetKeyboardState( &m_keyCount );

    m_lastKeys.clear();
    m_lastKeys.resize( m_keyCount );
}

void CInput::Update()
{
    memcpy( m_lastKeys.data(), m_pCurrKeys, m_keyCount );
}
