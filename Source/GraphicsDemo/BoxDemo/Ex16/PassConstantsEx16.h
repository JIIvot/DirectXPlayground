#pragma once

#include <DirectXMath.h>

struct PassConstantsEx16
{
    DirectX::XMFLOAT4X4 viewProjection;
    DirectX::XMFLOAT4   pulseColor;
    float               time;
};
