#include "Utility.h"
#include "ApplicationConstants.h"

#include <filesystem>
#include <cassert>
#include <wrl/client.h>
#include <string>
#include <Windows.h>
#include <d3dx12.h>

namespace Utility
{
IDxcBlob* CompileShaderFromFile( const wchar_t* pFilePath, uint32_t numCompileArgs, const wchar_t** ppCompileArgs )
{
    static Microsoft::WRL::ComPtr<IDxcUtils>          s_pUtils;
    static Microsoft::WRL::ComPtr<IDxcCompiler3>      s_pCompiler;
    static Microsoft::WRL::ComPtr<IDxcIncludeHandler> s_pIncludeHandler;

    if ( !std::filesystem::exists( pFilePath ) )
    {
        const std::wstring errorMessage = std::wstring( L"Failed to compile the shader! Path: " ) + pFilePath;

        MessageBox( nullptr, errorMessage.c_str(), kWindowTitle, MB_ICONERROR );
        assert( false );

        return nullptr;
    }

    if ( !s_pCompiler )
    {
        assert( !s_pUtils );
        assert( !s_pIncludeHandler );

        ASSERT_SUCCEEDED( DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( s_pUtils.GetAddressOf() ) ) );
        ASSERT_SUCCEEDED( DxcCreateInstance( CLSID_DxcCompiler, IID_PPV_ARGS( s_pCompiler.GetAddressOf() ) ) );
        ASSERT_SUCCEEDED( s_pUtils->CreateDefaultIncludeHandler( s_pIncludeHandler.GetAddressOf() ) );
    }

    uint32_t                                 codePage = CP_UTF8;
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSourceBlob;
    ASSERT_SUCCEEDED( s_pUtils->LoadFile( pFilePath, &codePage, pSourceBlob.GetAddressOf() ) );

    DxcBuffer sourceBuffer = {};
    sourceBuffer.Ptr = pSourceBlob->GetBufferPointer();
    sourceBuffer.Size = pSourceBlob->GetBufferSize();
    sourceBuffer.Encoding = codePage;

    Microsoft::WRL::ComPtr<IDxcResult> pResult;
    ASSERT_SUCCEEDED( s_pCompiler->Compile( &sourceBuffer, ppCompileArgs, numCompileArgs, s_pIncludeHandler.Get(), IID_PPV_ARGS( pResult.GetAddressOf() ) ) );

#ifdef _DEBUG
    Microsoft::WRL::ComPtr<IDxcBlobWide> pErrorMessages;
    ASSERT_SUCCEEDED( pResult->GetOutput( DXC_OUT_ERRORS, IID_PPV_ARGS( pErrorMessages.GetAddressOf() ), nullptr ) );

    if ( pErrorMessages && pErrorMessages->GetStringLength() > 0 )
    {
        static constexpr wchar_t kPlaceHolderFileName[] = L"hlsl.hlsl";

        std::wstring errorMessage = pErrorMessages->GetStringPointer();
        errorMessage.replace( errorMessage.find( kPlaceHolderFileName ), std::size( kPlaceHolderFileName ), pFilePath );

        std::printf( "%ws\n", errorMessage.c_str() );
        assert( false );
    }
#endif

    IDxcBlob* pDxilBlob = nullptr;
    ASSERT_SUCCEEDED( pResult->GetOutput( DXC_OUT_OBJECT, IID_PPV_ARGS( &pDxilBlob ), nullptr ) );

    return pDxilBlob;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateDefaultPsoDesc(
    DXGI_FORMAT                     rtvFormat,
    DXGI_FORMAT                     dsvFormat,
    size_t                          numInputElements,
    const D3D12_INPUT_ELEMENT_DESC* pInputLayout,
    ID3D12RootSignature*            pRootSignature,
    IDxcBlob*                       pVertexShaderBlob,
    IDxcBlob*                       pPixelShaderBlob )
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature = pRootSignature;
    desc.VS = GetShaderByteCodeFromBlob( pVertexShaderBlob );
    desc.PS = GetShaderByteCodeFromBlob( pPixelShaderBlob );
    desc.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    desc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
    desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC( D3D12_DEFAULT );
    desc.SampleMask = UINT_MAX;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = rtvFormat;
    desc.DSVFormat = dsvFormat;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    return desc;
}
} // namespace Utility
