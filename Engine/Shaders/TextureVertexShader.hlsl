struct VSInput {
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PSInput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 worldPosition : POSITION;
};

cbuffer constants : register(b0)
{
    row_major float4x4 MVP;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
    row_major float4x4 M;
}

PSInput main(VSInput input)
{
    PSInput output;
    output.position = mul(float4(input.position, 1.0f), MVP);
    output.texCoord = input.texCoord;
    output.worldPosition = mul(float4(input.position, 1.0f), M);
    
    return output;
}
