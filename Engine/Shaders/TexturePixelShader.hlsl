Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer SubUVConstant : register(b1)
{
    float indexU;
    float indexV;
}

cbuffer UUIDConstant : register(b2)
{
    float4 UUID;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 worldPosition : POSITION;
};

struct PSOutput
{
    float4 color : SV_Target0;
    float4 UUID : SV_Target1;
    float4 worldPos : SV_Target2;
};

PSOutput main(PSInput input) : SV_TARGET
{
    float2 uv = input.texCoord + float2(indexU, indexV);
    float4 col = gTexture.Sample(gSampler, uv);
    float threshold = 0.1; // 필요한 경우 임계값을 조정
    if (col.a < threshold)
        clip(-1); // 픽셀 버리기

    PSOutput output;
    output.color = col;
    output.UUID = -1;
    output.worldPos = float4(input.worldPosition, 1);
    return output;
}
