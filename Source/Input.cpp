#include "Input.h"

#include <cassert>

void Input::OnKeyDown( size_t key )
{
    assert( key < kNumKeys );
    m_currKeys[key] = true;
}

void Input::OnKeyUp( size_t key )
{
    assert( key < kNumKeys );
    m_currKeys[key] = false;
}

void Input::OnUpdateFinished()
{
    m_lastKeys = m_currKeys;
}

bool Input::IsKeyPressed( size_t key ) const
{
    assert( key < kNumKeys );
    return m_currKeys[key];
}

bool Input::IsKeyJustPressed( size_t key ) const
{
    assert( key < kNumKeys );
    return m_currKeys[key] && !m_lastKeys[key];
}
