cbuffer cbufferObject : register( b0 )
{
    float4x4 g_world;
}

cbuffer cbufferPass : register( b1 )
{
    float4x4 g_viewProjection;
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
    input.posL.xy += 0.5f * sin( input.posL.x ) * sin( 3.0f * g_time );
    input.posL.z *= 0.6f + 0.4f * sin( 2.0f * g_time );
    
    VertexOut output;
    
    const float4 posW = mul( float4( input.posL, 1.0f ), g_world );
    
    output.posH = mul( posW, g_viewProjection );
    output.color = input.color;
    
    return output;
}

float4 PS( VertexOut input ) : SV_Target
{
    return input.color;
}
