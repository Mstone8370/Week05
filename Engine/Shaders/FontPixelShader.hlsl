#include "ShaderRegisters.hlsl"

Texture2D Texture : register(t0);

cbuffer SubUVConstant : register(b3)
{
    float IndexU;
    float IndexV;
    float2 SubUVPadding;
}

struct PS_Input
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float4 main(PS_Input input) : SV_TARGET
{
    float2 UV = input.texCoord + float2(IndexU, IndexV);
    float4 Color = Texture.Sample(Sampler, UV);

    float Threshold = 0.05; // 필요한 경우 임계값을 조정
    if (Color.r < Threshold && Color.g < Threshold && Color.b < Threshold)
    {
        clip(-1); // 픽셀 버리기
    }

    return Color;
}
