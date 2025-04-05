#include "ShaderRegisters.hlsl"

struct VS_Input
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PS_Input
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

PS_Input main(VS_Input input)
{
    PS_Input output;
    output.position = float4(input.position, 1.f);
    output.position = mul(output.position, ModelMatrix);
    output.position = mul(output.position, ViewMatrix);
    output.position = mul(output.position, ProjectionMatrix);

    output.texCoord = input.texCoord;

    return output;
}
