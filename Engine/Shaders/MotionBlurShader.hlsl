#include "ShaderRegisters.hlsl"

Texture2D SceneBuffer : register(t0);
Texture2D VelocityBuffer : register(t1);
Texture2D NormalizedDepthBuffer : register(t2);
Texture2D ViewNormalBuffer : register(t3);

cbuffer MotionBlurConstants : register(b3)
{
    float2 ScreenSize;       // 화면 크기
    float  MaxBlurPixels;    // 최대 블러 거리 (픽셀 단위)
    float  VelocityScale;
    float  DepthThreshold;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    float2 uv = input.texCoord;

    float2 VelocityNDC = VelocityBuffer.Sample(Sampler, uv).rg;
    float2 VelocityPixel = VelocityNDC * 0.5f * ScreenSize;

    float2 BlurDir = (length(VelocityPixel) > 1.f) ? normalize(VelocityPixel) : float2(0, 0);

    VelocityPixel *= VelocityScale;
    float BlurLen = length(VelocityPixel);
    float BlurAmount = saturate(BlurLen);

    float PixelBlurLen = BlurAmount * MaxBlurPixels;     // 실제 픽셀 기준 블러 길이


    float3 NormalVS = ViewNormalBuffer.Sample(Sampler, uv).xyz * 2.0f - 1.0f; // Remap [0,1] → [-1,1]

    // Blur 방향은 XY만 고려하므로, normal의 XY만 사용
    float2 Normal2D = normalize(NormalVS.xy);

    float frontFactor = saturate(NormalVS.z);
    float directionFactor = saturate(dot(normalize(NormalVS.xy), BlurDir));
    BlurAmount *= frontFactor * directionFactor;

    const int Samples = 8;
    float3 Accum = 0.0;
    float WeightSum = 0.0;

    for (int i = -Samples; i <= Samples; i++)
    {
        float offset = i / float(Samples);
        float2 pixelOffset = BlurDir * offset * PixelBlurLen;
        float2 sampleUV = uv + pixelOffset / ScreenSize;  // pixel offset → UV offset

        float3 Color = SceneBuffer.Sample(Sampler, sampleUV).rgb;
        float Weight = 1.0 - abs(offset);

        Accum += Color * Weight;
        WeightSum += Weight;
    }

    float3 BlurredColor = Accum / WeightSum;

    // 3. Return: RGB = Blur Color, A = Blur Factor
    return float4(BlurredColor, BlurAmount);
}