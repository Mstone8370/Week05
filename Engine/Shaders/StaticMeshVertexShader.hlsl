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
    float3 WorldPos : TEXCOORD2; // 월드 공간 좌표 추가
    float3 CameraPos : TEXCOORD3;
    float3x3 TBN : TBN;
    float2 Velocity : TEXCOORD4;
    float3 ViewNormal : TEXCOORD5;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;

    output.materialIndex = input.materialIndex;

    // 위치 변환
    output.position = float4(input.position, 1.0);
    float4 worldPosition = mul(output.position, ModelMatrix);
    output.CameraPos = float3(InvViewMatrix._41, InvViewMatrix._42, InvViewMatrix._43);
    output.WorldPos = worldPosition.xyz;
    output.position = mul(worldPosition, ViewMatrix);
    output.position = mul(output.position, ProjectionMatrix);
    output.color = input.color;

    if (IsSelected)
    {
        output.color *= 0.5;
    }

    output.normal = mul(input.normal, (float3x3)InverseTransposedModel);

    float3 BiTangent = cross(input.normal, input.Tangent);
    matrix<float, 3, 3> TBN = {
        input.Tangent.x, input.Tangent.y, input.Tangent.z,        // column 0
        BiTangent.x, BiTangent.y, BiTangent.z,                    // column 1
        input.normal.x, input.normal.y, input.normal.z            // column 2
    };
    //output.TBN = transpose(TBN);
    output.TBN = TBN;

    output.texcoord = input.texcoord;

    float4 PrevPosition = float4(input.position, 1.0);
    PrevPosition = mul(PrevPosition, PrevModelMatrix);
    PrevPosition = mul(PrevPosition, PrevViewMatrix);
    PrevPosition = mul(PrevPosition, PrevProjectionMatrix);

    float4 CurrClip = output.position;
    float4 PrevClip = PrevPosition;

    float2 CurrNDC = CurrClip.xy / CurrClip.w;
    float2 PrevNDC = PrevClip.xy / PrevClip.w;

    // 보간할 velocity
    output.Velocity = CurrNDC - PrevNDC;

    float3 WorldNormal = normalize(mul(float4(input.normal, 0.0), ModelMatrix).xyz);
    output.ViewNormal = mul(WorldNormal, (float3x3)ViewMatrix); // Rotation only

    //output.WorldPos = PrevNDC;

    return output;
}