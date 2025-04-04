Texture2D sceneTexture : register(t0);
SamplerState gSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    float2 uv = input.texCoord;
    float4 col = sceneTexture.Sample(gSampler, uv);
    
    return col;
}