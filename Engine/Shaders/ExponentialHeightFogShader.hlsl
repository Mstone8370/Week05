#include "ShaderRegisters.hlsl"

Texture2D depthTexture : register(t0);
Texture2D worldPosTexture : register(t1);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 worldPosition : POSITION;
};

cbuffer FogConstants : register(b3)
{
    float3 fogColor; // 네가 원하는 색으로 바꿔도 됨
    float fogDensity;
    float fogFalloff;
    float fogHeight;
    float2 padding;
}

float3 ReconstructWorldPos(float2 uv, float depth)
{
    float4 ndc;
    ndc.xy = uv * 2.0 - 1.0;  // [0,1] → [-1,1]
    ndc.y *= -1;
    ndc.z = depth;
    ndc.w = 1.0;

    float4 worldPos = mul(ndc, InvProjectionMatrix);
    worldPos /= worldPos.w;

    worldPos = mul(worldPos, InvViewMatrix);
    //worldPos /= worldPos.w;

    return worldPos.xyz;
}

float ComputeFogFactor(float3 worldPos)
{
    // 높이 기반 안개 계산 - 아래로 갈수록 짙어짐

    // 아래로 내려갈수록 heightDiff가 커지게 (기준보다 얼마나 아래인가?)
    float heightDiff = max(0.0f, fogHeight - worldPos.z);

    // 지수적으로 감쇠되는 안개 계산
    float fogFactor = 1.0 - exp(-heightDiff * fogFalloff);

    // 밀도 반영
    fogFactor *= fogDensity;

    // 안전한 클램핑
    fogFactor = saturate(fogFactor);

    return fogFactor;
}

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
    float depth = depthTexture.Sample(Sampler, input.texCoord).r;
    //float linearDepth = LinearizeDepth(depth, cameraNearPlane, cameraFarPlane);
    //linearDepth = NormalizeLinearDepth(linearDepth, cameraNearPlane, cameraFarPlane);

    float3 worldPos = ReconstructWorldPos(input.texCoord, depth);
    //worldPos /= 500;
    //float3 worldPos =  worldPosTexture.Sample(gSampler, input.texCoord).xyz;
    float fogFactor = ComputeFogFactor(worldPos);
    //fogFactor = 1;

    return float4(fogColor, fogFactor);
}