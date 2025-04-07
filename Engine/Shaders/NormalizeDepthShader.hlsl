Texture2D depthTexture : register(t0);
SamplerState gSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

cbuffer constants : register(b0)
{
    float cameraNearPlane;
    float cameraFarPlane;
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
    float2 uv = input.texCoord;
    float depth = depthTexture.Sample(gSampler, uv).r;
    float NormalDepth = LinearizeDepth(depth, cameraNearPlane, cameraFarPlane);
    NormalDepth = NormalizeLinearDepth(NormalDepth, cameraNearPlane, cameraFarPlane);
    //NormalDepth = depth;
    float4 col = float4(NormalDepth, NormalDepth, NormalDepth, 1.0f);
    
    return col;
}