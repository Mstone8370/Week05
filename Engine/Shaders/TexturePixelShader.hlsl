Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer SubUVConstant : register(b3)
{
    float IndexU;
    float IndexV;
    float2 SubUVPadding;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    float2 uv = input.texCoord + float2(IndexU, IndexV);
    float4 col = gTexture.Sample(gSampler, uv);
    float threshold = 0.1; // 필요한 경우 임계값을 조정
    if (col.a < threshold)
    {
        clip(-1); // 픽셀 버리기
    }

    return col;
}
