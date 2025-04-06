Texture2D sceneTexture : register(t0);
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
    return (2.0 * nearPlane) / (farPlane + nearPlane - depth * (farPlane - nearPlane));
}

float NormalizeLinearDepth(float linearDepth, float nearPlane, float farPlane) {
    //float NormalizeLinearDepth(float linearDepth, float nearPlane, float farPlane) {
        return saturate((linearDepth - nearPlane) / (farPlane - nearPlane));
    //}
}

float4 main(PSInput input) : SV_TARGET
{
    float2 uv = input.texCoord;
    float depth = sceneTexture.Sample(gSampler, uv).r;
    // depth = saturate((depth - 0.999) * 1000);
    // 0.1f ~ 1000000.0f
    //depth = (depth - 0.9) * 10;
    //float col = float4(depth, depth, depth, 1);
    float NormalDepth = LinearizeDepth(depth, cameraNearPlane, cameraFarPlane);
    NormalDepth = pow(NormalDepth, 2.2);
    //float NormalDepth = NormalizeLinearDepth(linearDepth, cameraNearPlane, cameraFarPlane);
    float4 col = float4(NormalDepth, NormalDepth, NormalDepth, 1.0f);
    
    return col;
}