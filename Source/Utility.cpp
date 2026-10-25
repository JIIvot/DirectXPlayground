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
IDxcBlob* CompileShaderFromFile( const wchar_t* filePath, uint32_t numCompileArgs, const wchar_t** compileArgs )
{
    static Microsoft::WRL::ComPtr<IDxcUtils>          s_utils;
    static Microsoft::WRL::ComPtr<IDxcCompiler3>      s_compiler;
    static Microsoft::WRL::ComPtr<IDxcIncludeHandler> s_includeHandler;

    if ( !std::filesystem::exists( filePath ) )
    {
        const std::wstring errorMessage = std::wstring( L"Failed to compile the shader! Path: " ) + filePath;
        MessageBox( nullptr, errorMessage.c_str(), kWindowTitle, MB_ICONERROR );

        return nullptr;
    }

    if ( !s_compiler )
    {
        assert( !s_utils );
        assert( !s_includeHandler );

        ASSERT_SUCCEEDED( DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( s_utils.GetAddressOf() ) ) );
        ASSERT_SUCCEEDED( DxcCreateInstance( CLSID_DxcCompiler, IID_PPV_ARGS( s_compiler.GetAddressOf() ) ) );
        ASSERT_SUCCEEDED( s_utils->CreateDefaultIncludeHandler( s_includeHandler.GetAddressOf() ) );
    }

    uint32_t                                 codePage = CP_UTF8;
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> sourceBlob;
    ASSERT_SUCCEEDED( s_utils->LoadFile( filePath, &codePage, sourceBlob.GetAddressOf() ) );

    DxcBuffer sourceBuffer = {};
    sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
    sourceBuffer.Size = sourceBlob->GetBufferSize();
    sourceBuffer.Encoding = codePage;

    Microsoft::WRL::ComPtr<IDxcResult> result;
    ASSERT_SUCCEEDED( s_compiler->Compile( &sourceBuffer, compileArgs, numCompileArgs, s_includeHandler.Get(), IID_PPV_ARGS( result.GetAddressOf() ) ) );

#if 0
    Microsoft::WRL::ComPtr<IDxcBlobWide> errorMessages;
    ASSERT_SUCCEEDED( result->GetOutput( DXC_OUT_ERRORS, IID_PPV_ARGS( errorMessages.GetAddressOf() ), nullptr ) );

    if ( errorMessages && errorMessages->GetStringLength() > 0 )
    {
        static constexpr wchar_t kPlaceHolderFileName[] = L"hlsl.hlsl";

        std::wstring errorMessage = errorMessages->GetStringPointer();
        errorMessage.replace( errorMessage.find( kPlaceHolderFileName ), std::size( kPlaceHolderFileName ), filePath );

        std::printf( "%ws\n", errorMessage.c_str() );
    }
#endif

    IDxcBlob* dxilBlob = nullptr;
    ASSERT_SUCCEEDED( result->GetOutput( DXC_OUT_OBJECT, IID_PPV_ARGS( &dxilBlob ), nullptr ) );

    return dxilBlob;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateDefaultPsoDesc(
    DXGI_FORMAT                     rtvFormat,
    DXGI_FORMAT                     dsvFormat,
    uint32_t                        numInputElements,
    const D3D12_INPUT_ELEMENT_DESC* inputLayout,
    ID3D12RootSignature*            rootSignature,
    IDxcBlob*                       vertexShaderBlob,
    IDxcBlob*                       pixelShaderBlob )
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature = rootSignature;
    desc.VS = GetShaderByteCodeFromBlob( vertexShaderBlob );
    desc.PS = GetShaderByteCodeFromBlob( pixelShaderBlob );
    desc.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    desc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
    desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC( D3D12_DEFAULT );
    desc.SampleMask = UINT_MAX;
    desc.InputLayout.NumElements = numInputElements;
    desc.InputLayout.pInputElementDescs = inputLayout;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = rtvFormat;
    desc.DSVFormat = dsvFormat;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    return desc;
}
} // namespace Utility
