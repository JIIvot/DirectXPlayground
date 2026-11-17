SamplerState g_defaultSampler : register( s0 );

Texture2D g_renderTexture : register( t0 );

struct VertexIn
{
    float2 posL     : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VertexOut
{
    float4 posH     : SV_Position;
    float2 texCoord : TEXCOORD;
};

VertexOut VS( VertexIn input )
{
    VertexOut output;
    
    output.posH = float4( input.posL.xy, 0.0f, 1.0f );
    output.texCoord = input.texCoord;
    
    return output;
}

float4 PS( VertexOut input ) : SV_Target
{
    return g_renderTexture.Sample( g_defaultSampler, input.texCoord );
}
