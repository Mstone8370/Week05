#include "ShaderRegisters.hlsl"

Texture2D VelocityBuffer : register(t0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    float2 uv = input.texCoord;
    float2 velocity = VelocityBuffer.Sample(Sampler, uv).rg;
    velocity = (velocity + float2(1, 1)) * 0.5f;
    float4 col = float4(velocity, 0, 1.0f);

    return col;
}