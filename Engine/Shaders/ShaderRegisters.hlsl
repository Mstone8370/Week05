
SamplerState Sampler : register(s0);

////////
/// 공용: 0 ~ 2
///////
cbuffer ObjectBuffer : register(b0)
{
    row_major matrix ModelMatrix;
    row_major matrix InverseTransposedModel;
    row_major matrix PrevModelMatrix;
    float4 UUID;
    bool IsSelected;
    float3 ObjectPadding;
};

cbuffer ViewBuffer : register(b1)
{
    row_major matrix ViewMatrix;
    row_major matrix InvViewMatrix;
    row_major matrix PrevViewMatrix;
    float3 ViewLocation;
    float ViewPadding;
}

cbuffer ProjectionBuffer : register(b2)
{
    row_major matrix ProjectionMatrix;
    row_major matrix InvProjectionMatrix;
    row_major matrix PrevProjectionMatrix;
    float NearClip;
    float FarClip;
    float2 ProjectionPadding;
}
