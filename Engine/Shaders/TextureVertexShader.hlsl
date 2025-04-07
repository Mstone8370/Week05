#include "ShaderRegisters.hlsl"

struct VSInput
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

PSInput main(VSInput input)
{
    PSInput output;
    output.position = float4(input.position, 1.f);
    output.position = mul(output.position, ModelMatrix);
    output.position = mul(output.position, ViewMatrix);
    output.position = mul(output.position, ProjectionMatrix);
    output.texCoord = input.texCoord;

    return output;
}
