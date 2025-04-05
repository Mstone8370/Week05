
struct FMaterial
{
    float3 DiffuseColor;
    float TransparencyScalar;
    float3 AmbientColor;
    float DensityScalar;
    float3 SpecularColor;
    float SpecularScalar;
    float3 EmissiveColor;
    float MaterialPad0;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 MVP;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};

cbuffer MatrixBuffer : register(b0)
{
    row_major float4x4 MVP;
};

cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

cbuffer SubUVConstant : register(b1)
{
    float indexU;
    float indexV;
}

cbuffer GridParametersData : register(b1)
{
    float GridSpacing;
    int GridCount; // 총 grid 라인 수
    float3 GridOrigin; // Grid의 중심
    float Padding;
};

cbuffer UUIDConstant : register(b2)
{
    float4 UUID;
}

cbuffer FlagConstants : register(b3)
{
    bool IsLit;
    float3 flagPad0;
}

cbuffer PrimitiveCounts : register(b3)
{
    int BoundingBoxCount; // 렌더링할 AABB의 개수
    int pad;
    int ConeCount; // 렌더링할 cone의 개수
    int pad1;
};

cbuffer SubMeshConstants : register(b4)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b5)
{
    float2 UVOffset;
    float2 TexturePad0;
}


