cbuffer objectConstBuffer : register( b0 )
{
    float4x4 g_world;
}

cbuffer passConstBuffer : register( b1 )
{
    float4x4 g_viewProjection;
    float4   g_pulseColor;
    float    g_time;
}

struct VertexIn
{
    float3 posL  : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 posH  : SV_Position;
    float4 color : COLOR;
};

VertexOut VS( VertexIn input )
{
    VertexOut output;
    
    const float4 posW = mul( float4( input.posL, 1.0f ), g_world );
    
    output.posH = mul( posW, g_viewProjection );
    output.color = input.color;
    
    return output;
}

float4 PS( VertexOut input ) : SV_Target
{
    static const float kPi = 3.14159265359f;
    
    const float  t = 0.5f * sin( 2.0f * g_time - 0.25f * kPi ) + 0.5f;
    const float4 newColor = lerp( input.color, g_pulseColor, t );
    
    return newColor;
}
