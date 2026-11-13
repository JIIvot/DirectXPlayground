#include "Utility.h"

#include <filesystem>
#include <cassert>
#include <Windows.h>
#include <d3dx12.h>

namespace Utility
{
Microsoft::WRL::ComPtr<IDxcBlob> CompileShaderFromFile( const wchar_t* filePath, uint32_t numCompileArgs, const wchar_t** compileArgs )
{
    static Microsoft::WRL::ComPtr<IDxcUtils>          s_utils;
    static Microsoft::WRL::ComPtr<IDxcCompiler3>      s_compiler;
    static Microsoft::WRL::ComPtr<IDxcIncludeHandler> s_includeHandler;

    if ( !std::filesystem::exists( filePath ) )
    {
        LOG_ERROR( "The shader is not found! Path: %ws\n", filePath );
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

    const DxcBuffer sourceBuffer = {
        .Ptr = sourceBlob->GetBufferPointer(),
        .Size = sourceBlob->GetBufferSize(),
        .Encoding = codePage
    };

    Microsoft::WRL::ComPtr<IDxcResult> result;

#ifdef _DEBUG
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> errorMessages;

    const HRESULT hr = s_compiler->Compile( &sourceBuffer, compileArgs, numCompileArgs, s_includeHandler.Get(), IID_PPV_ARGS( result.GetAddressOf() ) );
    ASSERT_SUCCEEDED( result->GetErrorBuffer( errorMessages.GetAddressOf() ) );

    if ( errorMessages->GetBufferSize() > 0 )
    {
        LOG_ERROR( "Shader compilation failed! Path: %ws Error:\n%s\n", filePath, static_cast<char*>( errorMessages->GetBufferPointer() ) );
        return nullptr;
    }

    ASSERT_SUCCEEDED( hr );
#else
    s_compiler->Compile( &sourceBuffer, compileArgs, numCompileArgs, s_includeHandler.Get(), IID_PPV_ARGS( result.GetAddressOf() ) );
#endif

    Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
    ASSERT_SUCCEEDED( result->GetOutput( DXC_OUT_OBJECT, IID_PPV_ARGS( shaderBlob.GetAddressOf() ), nullptr ) );

    return shaderBlob;
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
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {
        .pRootSignature = rootSignature,
        .VS = GetShaderByteCodeFromBlob( vertexShaderBlob ),
        .PS = GetShaderByteCodeFromBlob( pixelShaderBlob ),
        .BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT ),
        .SampleMask = UINT_MAX,
        .RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT ),
        .DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC( D3D12_DEFAULT ),
        .InputLayout = { .pInputElementDescs = inputElementDescs, .NumElements = numInputElements },
        .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
        .NumRenderTargets = 1,
        .RTVFormats = { rtvFormat },
        .DSVFormat = dsvFormat,
        .SampleDesc = { .Count = 1 }
    };

    return desc;
}
} // namespace Utility
