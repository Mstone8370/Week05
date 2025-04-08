Texture2D sceneTexture : register(t0);
Texture2D MotionBlurBuffer  : register(t1);
Texture2D fogTexture : register(t2);

SamplerState gSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    float2 uv = input.texCoord;
    float4 sceneColor = sceneTexture.Sample(gSampler, uv);
    float4 fogSample =  fogTexture.Sample(gSampler, uv);
    float4 MotionBlurSample = MotionBlurBuffer.Sample(gSampler, uv);

    float3 fogColor = fogSample.rgb;
    float fogFactor = fogSample.a;

    float3 MotionBlurColor = MotionBlurSample.rgb;
    float MotionBlurFactor = MotionBlurSample.a;


    float3 MotionBlurred = lerp(sceneColor.rgb, MotionBlurColor.rgb, MotionBlurFactor);

    //float4 finalColor = float4(sceneColor.rgb * (float3(1, 1, 1) - fog) + fog, sceneColor.a);
    float3 finalColor = lerp(MotionBlurred.rgb, fogColor.rgb, fogFactor);

    return float4(finalColor, 1.0f);
}