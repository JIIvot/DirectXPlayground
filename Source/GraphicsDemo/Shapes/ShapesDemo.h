#pragma once

#include "GraphicsDemo/GraphicsDemo.h"
#include "ShapesFrameResource.h"
#include "Mesh.h"
#include "ShapesRenderItem.h"
#include "ShapesRenderLayer.h"

#include <vector>
#include <d3d12.h>
#include <wrl/client.h>
#include <unordered_map>
#include <dxcapi.h>

namespace ShapesDemo
{
class Demo final : public GraphicsDemo
{
public:
    explicit Demo( const GraphicsDemoCreateInfo& info );

    void OnRender( const GameTimer& timer ) override;

private:
    void InitRootSignature();
    void InitShaders();
    void InitPipelineStates();

private:
    static constexpr int32_t kNumFramesInFlight = 3;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    std::unordered_map<std::string, Mesh>                                        m_meshMap;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<IDxcBlob>>            m_shaderMap;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_psoMap;

    std::vector<RenderItem>  m_renderItems;
    std::vector<RenderItem*> m_layerRenderItems[eRenderLayer_Count] = {};

    PassConstants m_passConstants;

    FrameResource  m_frameResources[kNumFramesInFlight];
    FrameResource* m_currFrameResource;
    int32_t        m_frameResourceIndex;

    DirectX::XMFLOAT4X4 m_view;
    DirectX::XMFLOAT4X4 m_projection;
    DirectX::XMFLOAT3   m_eyePosition;
    float               m_viewPolarAngle = DirectX::XM_PI * 1.25f;
    float               m_viewAzimuthalAngle = DirectX::XM_PIDIV4;
    float               m_viewRadius = 10.0f;

    bool m_wireframeEnabled = false;
};
} // namespace ShapesDemo
