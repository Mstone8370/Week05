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
    bool normalFlag : TEXCOORD0; // 노멀 유효성 플래그 (1.0: 유효, 0.0: 무효)
    float2 texcoord : TEXCOORD1;
    int materialIndex : MATERIAL_INDEX;
    float2 Velocity : TEXCOORD2;
    float2 WorldPos : TEXCOORD3;
    float3 ViewNormal   : TEXCOORD4;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;

    output.materialIndex = input.materialIndex;

    // 위치 변환
    output.position = float4(input.position, 1.0);
    output.position = mul(output.position, ModelMatrix);
    output.position = mul(output.position, ViewMatrix);
    output.position = mul(output.position, ProjectionMatrix);
    output.color = input.color;

    if (IsSelected)
    {
        output.color *= 0.5;
    }
    // 입력 normal 값의 길이 확인
    float normalThreshold = 0.001;
    float normalLen = length(input.normal);

    if (normalLen < normalThreshold)
    {
        output.normalFlag = 0.0;
    }
    else
    {
        //output.normal = normalize(input.normal);
        output.normal = normalize(mul(input.normal, InverseTranspose));
        output.normalFlag = 1.0;
    }
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