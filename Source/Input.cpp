#include "Input.h"

#include <cassert>

void CInput::OnKeyDown( size_t key )
{
    assert( key < kNumKeys );
    m_currKeys[key] = true;
}

void CInput::OnKeyUp( size_t key )
{
    assert( key < kNumKeys );
    m_currKeys[key] = false;
}

void CInput::OnFrameFinished()
{
    m_lastKeys = m_currKeys;
}

bool CInput::IsKeyPressed( size_t key ) const
{
    assert( key < kNumKeys );
    return m_currKeys[key];
}

bool CInput::IsKeyJustPressed( size_t key ) const
{
    assert( key < kNumKeys );
    return m_currKeys[key] && !m_lastKeys[key];
}
