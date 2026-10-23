#pragma once

#include "GraphicsDemo/GraphicsDemo.h"
#include "Input.h"

#include <cassert>
#include <memory>

struct SDL_Window;

class CApplication
{
public:
    CApplication( const CApplication& ) = delete;
    CApplication& operator=( const CApplication& ) = delete;

    CApplication( CApplication&& ) = delete;
    CApplication& operator=( CApplication&& ) = delete;

    template<typename TGraphicsDemo>
    static void Init();

    [[nodiscard]]
    static CApplication& GetInstance();

    static void Destroy();

    void Run();

    void Quit()
    {
        m_shouldRun = false;
    }

    [[nodiscard]]
    int32_t GetWidth() const
    {
        return m_clientWidth;
    }

    [[nodiscard]]
    int32_t GetHeight() const
    {
        return m_clientHeight;
    }

    [[nodiscard]]
    const CInput& GetInput() const
    {
        return m_input;
    }

private:
    CApplication();
    ~CApplication();

    [[nodiscard]]
    bool InitWindow();

    void ProcessEvents();
    void OnResize( int32_t newWidth, int32_t newHeight );

private:
    static constexpr char kWindowTitle[] = "DirectX 12 Engine";

    static CApplication* s_pInstance;

    bool m_shouldRun = false;

    CInput m_input;

    std::unique_ptr<CGraphicsDemo> m_pDemo;

    SDL_Window* m_pWindow = nullptr;
    HWND        m_hNativeWindow = nullptr;

    int32_t m_clientWidth = 1280;
    int32_t m_clientHeight = 720;
};

template<typename TGraphicsDemo>
void CApplication::Init()
{
    assert( !s_pInstance );

    s_pInstance = new CApplication;
    if ( s_pInstance->m_shouldRun )
    {
        s_pInstance->m_pDemo = std::make_unique<TGraphicsDemo>( s_pInstance->m_hNativeWindow );
    }
}
