#include "ShaderRegisters.hlsl"

struct VS_INPUT
{
    float3 position : POSITION; // 버텍스 위치
    float3 normal : NORMAL; // 버텍스 노멀
    float3 Tangent : TANGENT; // 버텍스 노멀
    float2 texcoord : TEXCOORD;
    float4 color : COLOR; // 버텍스 색상
    uint materialIndex : MATERIAL_INDEX;
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // 변환된 화면 좌표
    float4 color : COLOR; // 전달할 색상
    float3 normal : NORMAL; // 정규화된 노멀 벡터
    float2 texcoord : TEXCOORD1;
    int materialIndex : MATERIAL_INDEX;
    float3 worldPos : TEXCOORD2; // 월드 공간 좌표 추가
    float3 cameraPos : TEXCOORD3;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;

    output.materialIndex = input.materialIndex;

    // 위치 변환
    output.position = float4(input.position, 1.0);
    float4 worldPosition = mul(output.position, ModelMatrix);
    output.cameraPos = float3(InvViewMatrix._41, InvViewMatrix._42, InvViewMatrix._43);
    output.worldPos = worldPosition.xyz;
    output.position = mul(worldPosition, ViewMatrix);
    output.position = mul(output.position, ProjectionMatrix);
    output.color = input.color;

    if (IsSelected)
    {
        output.color *= 0.5;
    }

    output.normal = mul(input.normal, (float3x3)ModelMatrix);
    output.texcoord = input.texcoord;

    return output;
}