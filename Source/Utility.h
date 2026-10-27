#pragma once

#include <cstdint>
#include <d3d12.h>
#include <dxcapi.h>

#ifdef _DEBUG
    #include <Windows.h>
    #include <cstdio>

    #define ANSI_COLOR_RED "\x1b[31m"
    #define ANSI_COLOR_GREEN "\x1b[32m"
    #define ANSI_COLOR_YELLOW "\x1b[33m"
    #define ANSI_COLOR_BLUE "\x1b[34m"
    #define ANSI_COLOR_RESET "\x1b[0m"

    #define LOG_TRACE( x, ... ) std::printf( ANSI_COLOR_BLUE "[TRACE]: " x ANSI_COLOR_RESET, __VA_ARGS__ )
    #define LOG_INFO( x, ... ) std::printf( ANSI_COLOR_GREEN "[INFO]:  " x ANSI_COLOR_RESET, __VA_ARGS__ )
    #define LOG_WARN( x, ... ) std::printf( ANSI_COLOR_YELLOW "[WARN]:  " x ANSI_COLOR_RESET, __VA_ARGS__ )
    #define LOG_ERROR( x, ... ) std::printf( ANSI_COLOR_RED "[ERROR]: " x ANSI_COLOR_RESET, __VA_ARGS__ )

    #define MAKE_STRING_INTERNAL( x ) #x
    #define MAKE_STRING( x ) MAKE_STRING_INTERNAL( x )

    #define ASSERT_SUCCEEDED( hr )                                                                              \
        if ( FAILED( hr ) )                                                                                     \
        {                                                                                                       \
            LOG_ERROR( "HRESULT failed in " MAKE_STRING( __FILE__ ) " at line " MAKE_STRING( __LINE__ ) "\n" ); \
            LOG_ERROR( "hr = 0x%08X\n", hr );                                                                   \
            __debugbreak();                                                                                     \
        }
#else
    #define LOG_TRACE( x, ... )
    #define LOG_INFO( x, ... )
    #define LOG_WARN( x, ... )
    #define LOG_ERROR( x, ... )

    #define ASSERT_SUCCEEDED( hr, ... ) ( void )( hr )
#endif

namespace Utility
{
[[nodiscard]]
IDxcBlob* CompileShaderFromFile( const wchar_t* filePath, uint32_t numCompileArgs, const wchar_t** compileArgs );

[[nodiscard]]
inline D3D12_SHADER_BYTECODE GetShaderByteCodeFromBlob( IDxcBlob* blob )
{
    return { .pShaderBytecode = blob->GetBufferPointer(), .BytecodeLength = blob->GetBufferSize() };
}

[[nodiscard]]
D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateDefaultPsoDesc(
    DXGI_FORMAT                     rtvFormat,
    DXGI_FORMAT                     dsvFormat,
    uint32_t                        numInputElements,
    const D3D12_INPUT_ELEMENT_DESC* inputLayout,
    ID3D12RootSignature*            rootSignature,
    IDxcBlob*                       vertexShaderBlob,
    IDxcBlob*                       pixelShaderBlob );
} // namespace Utility
