cbuffer cbufferObject : register( b0 )
{
    float4x4 g_world;
}

cbuffer cbufferPass : register( b1 )
{
    float4x4 g_viewProjection;
}

struct VertexIn
{
    float3 PosL  : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH  : SV_Position;
    float4 Color : COLOR;
};

VertexOut VS( VertexIn input )
{
    VertexOut output;
    
    const float4 posW = mul( float4( input.PosL, 1.0f ), g_world );
    
    output.PosH = mul( posW, g_viewProjection );
    output.Color = input.Color;
    
    return output;
}

float4 PS( VertexOut input ) : SV_Target
{
    return input.Color;
}
