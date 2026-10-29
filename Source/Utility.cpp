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
void CompileShaderFromFile( const wchar_t* filePath, uint32_t numCompileArgs, const wchar_t** compileArgs, IDxcBlob** shaderBlob )
{
    static Microsoft::WRL::ComPtr<IDxcUtils>          s_utils;
    static Microsoft::WRL::ComPtr<IDxcCompiler3>      s_compiler;
    static Microsoft::WRL::ComPtr<IDxcIncludeHandler> s_includeHandler;

    if ( !std::filesystem::exists( filePath ) )
    {
        LOG_ERROR( "The shader is not found! Path: %ws\n", filePath );
        return;
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

#ifdef _DEBUG
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> errorMessages;

    const HRESULT hr = s_compiler->Compile( &sourceBuffer, compileArgs, numCompileArgs, s_includeHandler.Get(), IID_PPV_ARGS( result.GetAddressOf() ) );
    result->GetErrorBuffer( errorMessages.GetAddressOf() );

    if ( errorMessages->GetBufferSize() > 0 )
    {
        LOG_ERROR( "Shader compilation failed! Path: %ws Error:\n%s\n", filePath, static_cast<char*>( errorMessages->GetBufferPointer() ) );
        return;
    }

    ASSERT_SUCCEEDED( hr );
#else
    s_compiler->Compile( &sourceBuffer, compileArgs, numCompileArgs, s_includeHandler.Get(), IID_PPV_ARGS( result.GetAddressOf() ) );
#endif

    ASSERT_SUCCEEDED( result->GetOutput( DXC_OUT_OBJECT, IID_PPV_ARGS( shaderBlob ), nullptr ) );
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateDefaultPsoDesc(
    DXGI_FORMAT                     rtvFormat,
    DXGI_FORMAT                     dsvFormat,
    uint32_t                        numInputElements,
    const D3D12_INPUT_ELEMENT_DESC* inputElementDescs,
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
    desc.InputLayout.pInputElementDescs = inputElementDescs;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = rtvFormat;
    desc.DSVFormat = dsvFormat;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    return desc;
}
} // namespace Utility
