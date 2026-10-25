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

VertexOut VS(VertexIn input)
{
    VertexOut output;
    output.PosH = float4(input.PosL, 1.0);
    output.Color = input.Color;
    
    return output;
}

float4 PS(VertexOut input) : SV_Target
{
    return input.Color;
}
