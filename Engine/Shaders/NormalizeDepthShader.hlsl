#include "ShaderRegisters.hlsl"

Texture2D depthTexture : register(t0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float LinearizeDepth(float depth, float nearPlane, float farPlane)
{
    return (farPlane * nearPlane) / (farPlane - depth * (farPlane - nearPlane));
}

float NormalizeLinearDepth(float linearDepth, float nearPlane, float farPlane)
{
    return saturate((linearDepth - nearPlane) / (farPlane - nearPlane));
}

float4 main(PSInput input) : SV_TARGET
{
    float2 uv = input.texCoord;
    float depth = depthTexture.Sample(Sampler, uv).r;
    float NormalDepth = LinearizeDepth(depth, NearClip, FarClip);
    NormalDepth = NormalizeLinearDepth(NormalDepth, NearClip, FarClip);
    //NormalDepth = depth;
    float4 col = float4(NormalDepth, NormalDepth, NormalDepth, 1.0f);

    return col;
}