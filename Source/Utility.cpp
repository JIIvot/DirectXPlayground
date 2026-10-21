#include "Utility.h"

#include <filesystem>
#include <cassert>
#include <string>

namespace Utility
{
Microsoft::WRL::ComPtr<IDxcBlob> CompileShaderFromFile( const wchar_t* pFileName, uint32_t numCompileArgs, const wchar_t** ppCompileArgs )
{
    static Microsoft::WRL::ComPtr<IDxcUtils>          pUtils;
    static Microsoft::WRL::ComPtr<IDxcCompiler3>      pCompiler;
    static Microsoft::WRL::ComPtr<IDxcIncludeHandler> pIncludeHandler;

    assert( std::filesystem::exists( pFileName ) );

    if ( !pCompiler )
    {
        assert( !pUtils );
        assert( !pIncludeHandler );

        ASSERT_SUCCEEDED( DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( pUtils.GetAddressOf() ) ) );
        ASSERT_SUCCEEDED( DxcCreateInstance( CLSID_DxcCompiler, IID_PPV_ARGS( pCompiler.GetAddressOf() ) ) );
        ASSERT_SUCCEEDED( pUtils->CreateDefaultIncludeHandler( pIncludeHandler.GetAddressOf() ) );
    }

    uint32_t                                 codePage = CP_UTF8;
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSourceBlob;
    ASSERT_SUCCEEDED( pUtils->LoadFile( pFileName, &codePage, pSourceBlob.GetAddressOf() ) );

    const DxcBuffer sourceBuffer = {
        .Ptr = pSourceBlob->GetBufferPointer(),
        .Size = pSourceBlob->GetBufferSize(),
        .Encoding = codePage
    };

    // TODO: Do a proper separation in the logic between debug and release builds.

    Microsoft::WRL::ComPtr<IDxcResult> pResult;

#ifdef _DEBUG
    HRESULT hr = pCompiler->Compile(
        &sourceBuffer,         // pSource
        ppCompileArgs,         // pArguments
        numCompileArgs,        // argCount
        pIncludeHandler.Get(), // pIncludeHandler
        IID_PPV_ARGS( pResult.GetAddressOf() ) );

    if ( SUCCEEDED( hr ) )
    {
        ASSERT_SUCCEEDED( pResult->GetStatus( &hr ) );
    }

    Microsoft::WRL::ComPtr<IDxcBlobWide> pErrorMessages;
    ASSERT_SUCCEEDED( pResult->GetOutput( DXC_OUT_ERRORS, IID_PPV_ARGS( pErrorMessages.GetAddressOf() ), nullptr ) );

    if ( pErrorMessages && pErrorMessages->GetStringLength() > 0 )
    {
        std::wstring errorText = pErrorMessages->GetStringPointer();

        static constexpr wchar_t kPlaceHolderFileName[] = L"hlsl.hlsl";
        errorText.replace( errorText.find( kPlaceHolderFileName ), std::size( kPlaceHolderFileName ), pFileName );

        std::wprintf( errorText.c_str() );
        ASSERT_SUCCEEDED( E_FAIL );
    }
#else
    ASSERT_SUCCEEDED( pCompiler->Compile(
        &sourceBuffer,         // pSource
        ppCompileArgs,         // pArguments
        numCompileArgs,        // argCount
        pIncludeHandler.Get(), // pIncludeHandler
        IID_PPV_ARGS( pResult.GetAddressOf() ) ) );
#endif

    Microsoft::WRL::ComPtr<IDxcBlob> pDxilBlob;
    ASSERT_SUCCEEDED( pResult->GetOutput( DXC_OUT_OBJECT, IID_PPV_ARGS( pDxilBlob.GetAddressOf() ), nullptr ) );

    // TODO: Create PDB files when _DEBUG is defined

    return pDxilBlob;
}
} // namespace Utility
