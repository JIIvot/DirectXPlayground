#include "Input.h"

#include <cassert>

Input::Input( int32_t initialMouseX, int32_t initialMouseY )
    : m_currMousePosition( static_cast<float>( initialMouseX ), static_cast<float>( initialMouseY ) )
    , m_lastMousePosition( m_currMousePosition )
{
}

void Input::OnKeyDown( int32_t key )
{
    assert( key < kNumKeys );
    m_currKeys[key] = true;
}

void Input::OnKeyUp( int32_t key )
{
    assert( key < kNumKeys );
    m_currKeys[key] = false;
}

void Input::OnMouseMove( int32_t mouseX, int32_t mouseY )
{
    m_currMousePosition.x = static_cast<float>( mouseX );
    m_currMousePosition.y = static_cast<float>( mouseY );
}

void Input::OnUpdateStarted()
{
    m_mouseDelta.x = m_currMousePosition.x - m_lastMousePosition.x;
    m_mouseDelta.y = m_currMousePosition.y - m_lastMousePosition.y;
}

void Input::OnUpdateFinished()
{
    m_lastKeys = m_currKeys;
    m_lastButtons = m_currButtons;
    m_lastMousePosition = m_currMousePosition;
}

bool Input::IsKeyPressed( int32_t key ) const
{
    assert( key < kNumKeys );
    return m_currKeys[key];
}

bool Input::IsKeyJustPressed( int32_t key ) const
{
    assert( key < kNumKeys );
    return m_currKeys[key] && !m_lastKeys[key];
}
