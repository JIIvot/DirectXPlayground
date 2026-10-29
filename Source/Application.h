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

    [[nodiscard]] static CApplication& Get();
    static void                        Destroy();

    void Run();

    void Quit() const
    {
        PostMessage( m_windowHandle, WM_CLOSE, 0, 0 );
    }

    [[nodiscard]] int32_t GetWidth() const
    {
        return m_clientWidth;
    }

    [[nodiscard]] int32_t GetHeight() const
    {
        return m_clientHeight;
    }

    [[nodiscard]] const CInput& GetInput() const
    {
        return m_input;
    }

private:
    CApplication();

    static LRESULT CALLBACK WindowProcedure( HWND windowHandle, uint32_t message, WPARAM wParam, LPARAM lParam );
    LRESULT                 ProcessWindowMessage( HWND windowHandle, uint32_t message, WPARAM wParam, LPARAM lParam );

    [[nodiscard]] bool InitWindow();
    void               CreateGraphicsContext();

private:
    static CApplication* s_instance;

    HWND                           m_windowHandle = nullptr;
    std::unique_ptr<CGraphicsDemo> m_demo;

    Microsoft::WRL::ComPtr<IDXGIFactory6> m_dxgiFactory;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> m_dxgiAdapter;
    Microsoft::WRL::ComPtr<ID3D12Device5> m_device;

    CInput m_input;

    int32_t m_clientWidth = 1280;
    int32_t m_clientHeight = 720;

    bool m_isPaused = false;
    bool m_shouldResize = false;
};

template<typename TGraphicsDemo>
void CApplication::Init()
{
    assert( !s_instance );

    s_instance = new CApplication;
    if ( s_instance->m_windowHandle )
    {
        s_instance->m_demo = std::make_unique<TGraphicsDemo>( s_instance->m_windowHandle, s_instance->m_dxgiFactory.Get(), s_instance->m_device.Get() );
    }
}
