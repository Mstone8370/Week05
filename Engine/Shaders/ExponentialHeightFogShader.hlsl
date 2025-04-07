#include "ShaderRegisters.hlsl"

Texture2D DepthTexture : register(t0);

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 WorldPosition : POSITION;
};

cbuffer FogConstants : register(b3)
{
    float3 FogColor; // 네가 원하는 색으로 바꿔도 됨
    float FogDensity;
    float FogFalloff;
    float FogHeight;

    float FogStartDistance;     // 안개 시작 거리
    float FogEndDistance;       // 안개 끝 거리
    float DistanceFogIntensity; // 거리 기반 안개 영향
    float3 Padding345;
}

float3 ReconstructWorldPos(float2 UV, float Depth)
{
    float4 NDC;
    NDC.xy = UV * 2.0 - 1.0;  // [0,1] → [-1,1]
    NDC.y *= -1;
    NDC.z = Depth;
    NDC.w = 1.0;

    float4 WorldPos = mul(NDC, InvProjectionMatrix);
    WorldPos /= WorldPos.w;

    WorldPos = mul(WorldPos, InvViewMatrix);

    return WorldPos.xyz;
}

float GetCameraDistance(float Depth)
{
    float Z = Depth;
    float4 ClipPos = float4(0, 0, Z, 1);
    float4 ViewPos = mul(ClipPos, InvProjectionMatrix);
    float ViewZ = ViewPos.z / ViewPos.w;
    return abs(ViewZ); // 카메라 기준 거리
}

float ComputeFogFactor(float3 WorldPos, float Depth)
{
    float3 CameraPos = InvViewMatrix[3].xyz;

    float CameraDist = GetCameraDistance(Depth);
    float DistFactor = saturate((CameraDist - FogStartDistance) / (FogEndDistance - FogStartDistance));

    // 높이 기반 안개
    float HeightDiff = max(0.0f, FogHeight - WorldPos.z);
    float HeightFactor = 1.0 - exp(-HeightDiff * FogFalloff);
    float HeightFogFactor = HeightFactor * FogDensity * DistFactor; // <- 거리 영향 추가됨



    // 거리 기반 안개 (카메라가 안개 안에 있을 때만)
    float CameraHeightDiff = max(0.0f, FogHeight - CameraPos.z);
    float CameraHeightFactor = 1.0 - exp(-CameraHeightDiff * FogFalloff);

    float DistFogFactor = DistFactor * FogDensity * DistanceFogIntensity * (CameraHeightFactor);


    float FogFactor = HeightFogFactor + DistFogFactor;
    // 안전한 클램핑
    FogFactor = saturate(FogFactor);

    return FogFactor;
}

float LinearizeDepth(float Depth, float NearPlane, float FarPlane)
{
    return (FarPlane * NearPlane) / (FarPlane - Depth * (FarPlane - NearPlane));
}

float NormalizeLinearDepth(float LinearDepth, float NearPlane, float FarPlane)
{
    return saturate((LinearDepth - NearPlane) / (FarPlane - NearPlane));
}

float4 main(PSInput Input) : SV_TARGET
{
    float Depth = DepthTexture.Sample(Sampler, Input.TexCoord).r;

    float3 WorldPos = ReconstructWorldPos(Input.TexCoord, Depth);

    float FogFactor = ComputeFogFactor(WorldPos, Depth);


    return float4(FogColor, FogFactor);
}