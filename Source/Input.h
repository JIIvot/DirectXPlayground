#pragma once

#include <bitset>
#include <DirectXMath.h>

class Input
{
public:
    enum MouseButton : uint8_t
    {
        eMouseButton_Left = 0,
        eMouseButton_Right,
        eMouseButton_Middle,
        eMouseButton_Count
    };

public:
    Input( int32_t initalMouseX, int32_t initialMouseY );

    void OnKeyDown( int32_t key );
    void OnKeyUp( int32_t key );
    void OnButtonDown( MouseButton button );
    void OnButtonUp( MouseButton button );
    void OnMouseMove( int32_t mouseX, int32_t mouseY );

    void OnUpdateStarted();
    void OnUpdateFinished();

    [[nodiscard]] bool IsKeyPressed( int32_t key ) const;
    [[nodiscard]] bool IsKeyJustPressed( int32_t key ) const;
    [[nodiscard]] bool IsButtonPressed( MouseButton button ) const;
    [[nodiscard]] bool IsButtonJustPressed( MouseButton button ) const;

    [[nodiscard]] DirectX::XMFLOAT2 GetMousePosition() const
    {
        return m_currMousePosition;
    }

    [[nodiscard]] DirectX::XMFLOAT2 GetMouseDelta() const
    {
        return m_mouseDelta;
    }

private:
    static constexpr int32_t kNumKeys = 256;

    std::bitset<kNumKeys>           m_currKeys;
    std::bitset<kNumKeys>           m_lastKeys;
    std::bitset<eMouseButton_Count> m_currButtons;
    std::bitset<eMouseButton_Count> m_lastButtons;

    DirectX::XMFLOAT2 m_currMousePosition;
    DirectX::XMFLOAT2 m_lastMousePosition;
    DirectX::XMFLOAT2 m_mouseDelta = {};
};
