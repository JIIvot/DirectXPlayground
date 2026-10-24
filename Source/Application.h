#pragma once

#include "GraphicsDemo/GraphicsDemo.h"
#include "Input.h"

#include <cassert>
#include <memory>

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

    static LRESULT CALLBACK WindowProcedure( HWND hWindow, uint32_t message, WPARAM wParam, LPARAM lParam );
    LRESULT                 ProcessWindowMessage( HWND hWindow, uint32_t message, WPARAM wParam, LPARAM lParam );

    [[nodiscard]]
    bool InitWindow();

    void OnResize( int32_t newWidth, int32_t newHeight );

private:
    static CApplication* s_pInstance;

    HWND                           m_hWindow = nullptr;
    std::unique_ptr<CGraphicsDemo> m_pDemo;
    CInput                         m_input;

    int32_t m_clientWidth = 1280;
    int32_t m_clientHeight = 720;

    bool m_shouldRun = false;
};

template<typename TGraphicsDemo>
void CApplication::Init()
{
    assert( !s_pInstance );

    s_pInstance = new CApplication;
    if ( s_pInstance->m_shouldRun )
    {
        s_pInstance->m_pDemo = std::make_unique<TGraphicsDemo>( s_pInstance->m_hWindow );
    }
}
