#pragma once

#include <cstdint>
#include <d3d12.h>
#include <dxcapi.h>

#ifdef _DEBUG
    #include <Windows.h>
    #include <cstdio>

    #define MAKE_STRING_INTERNAL( x ) #x
    #define MAKE_STRING( x ) MAKE_STRING_INTERNAL( x )

    #define ASSERT_SUCCEEDED( hr, ... )                                                                           \
        if ( FAILED( hr ) )                                                                                       \
        {                                                                                                         \
            std::printf( "HRESULT failed in " MAKE_STRING( __FILE__ ) " at line " MAKE_STRING( __LINE__ ) "\n" ); \
            std::printf( "hr = 0x%08X\n", hr );                                                                   \
            std::printf( __VA_ARGS__ "\n" );                                                                      \
            __debugbreak();                                                                                       \
        }
#else
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
    size_t                          numInputElements,
    const D3D12_INPUT_ELEMENT_DESC* inputLayout,
    ID3D12RootSignature*            rootSignature,
    IDxcBlob*                       vertexShaderBlob,
    IDxcBlob*                       pixelShaderBlob );
} // namespace Utility
