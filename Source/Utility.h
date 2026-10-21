#pragma once

#include <wrl/client.h>
#include <dxcapi.h>
#include <cstdint>
#include <d3d12.h>

#ifdef _DEBUG
    #include <Windows.h>

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
Microsoft::WRL::ComPtr<IDxcBlob> CompileShaderFromFile( const wchar_t* pFileName, uint32_t numCompileArgs, const wchar_t** ppCompileArgs );

[[nodiscard]]
inline D3D12_SHADER_BYTECODE GetShaderByteCodeFromBlob( IDxcBlob* blob )
{
    return { .pShaderBytecode = blob->GetBufferPointer(), .BytecodeLength = blob->GetBufferSize() };
}
} // namespace Utility
