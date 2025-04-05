
SamplerState Sampler : register(s0);

////////
/// 공용: 0 ~ 2
///////
cbuffer ObjectBuffer : register(b0)
{
    row_major matrix ModelMatrix;
    row_major matrix InverseTranspose;
    float4 UUID;
    bool IsSelected;
    float3 ObjectPadding;
};

cbuffer ViewBuffer : register(b1)
{
    row_major matrix ViewMatrix;
    float3 ViewLocation;
    float ViewPadding;
}

cbuffer ProjectionBuffer : register(b2)
{
    row_major matrix ProjectionMatrix;
    float NearClip;
    float FarClip;
    float2 ProjectionPadding;
}
