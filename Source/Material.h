#pragma once

#include <string>
#include <DirectXMath.h>

struct Material
{
    std::string name;
    int32_t     numDirtyFrames;

    int32_t materialBufferIndex;
    int32_t albedoTextureIndex;
    int32_t normalTextureIndex;
    int32_t glossHeightAoTextureIndex;

    DirectX::XMFLOAT4 diffuseAlbedo;
    DirectX::XMFLOAT4 normalIncidenceReflectance; // Frensel R0
    float             roughness;
    float             displacementScale;

    DirectX::XMFLOAT4X4 transform;

    // Ray-tracing only
    float transparencyWeight;
    float indexOfRefraction;
};
