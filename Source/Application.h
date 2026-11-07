#pragma once

#include "GraphicsDemo/GraphicsDemo.h"
#include "Input/Input.h"
#include "GameTimer.h"

#include <cassert>
#include <memory>
#include <optional>

class Application
{
public:
    Application( const Application& ) = delete;
    Application& operator=( const Application& ) = delete;

    Application( Application&& ) = delete;
    Application& operator=( Application&& ) = delete;

    template<typename TGraphicsDemo>
    static void Init();

    [[nodiscard]] static Application& Get();

    static void Destroy();

    void Run();

    void Quit() const
    {
        PostMessage( m_windowHandle, WM_CLOSE, 0, 0 );
    }

private:
    Application();

    static LRESULT CALLBACK WindowProcedure( HWND windowHandle, uint32_t message, WPARAM wParam, LPARAM lParam );
    LRESULT                 ProcessWindowMessage( HWND windowHandle, uint32_t message, WPARAM wParam, LPARAM lParam );

    [[nodiscard]] bool InitWindow();
    void               InitGraphics();

private:
    static Application* s_instance;

    HWND                          m_windowHandle = nullptr;
    std::unique_ptr<GraphicsDemo> m_demo;

    Microsoft::WRL::ComPtr<IDXGIFactory6> m_dxgiFactory;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> m_dxgiAdapter;
    Microsoft::WRL::ComPtr<ID3D12Device5> m_device;

    std::optional<Input> m_input;

    int32_t m_clientWidth = 1280;
    int32_t m_clientHeight = 720;

    GameTimer m_timer;

    bool m_isPaused = false;
    bool m_shouldResize = false;
};

template<typename TGraphicsDemo>
void Application::Init()
{
    assert( !s_instance );
    s_instance = new Application;

    const GraphicsDemoCreateInfo info = {
        .windowHandle = s_instance->m_windowHandle,
        .dxgiFactory = s_instance->m_dxgiFactory.Get(),
        .device = s_instance->m_device.Get(),
        .clientWidth = s_instance->m_clientWidth,
        .clientHeight = s_instance->m_clientHeight
    };

    s_instance->m_demo = std::make_unique<TGraphicsDemo>( info );
}
