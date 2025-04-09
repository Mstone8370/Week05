#include "ShaderRegisters.hlsl"

Texture2D DiffuseTexture : register(t0);
Texture2D EmissiveTexture : register(t1);
Texture2D RoughnessTexture : register(t2);
Texture2D NormalTexture : register(t2);

cbuffer FlagConstants : register(b3)
{
    bool IsLit;
    float3 FlagPadding;
}

struct FMaterial
{
    float3 DiffuseColor;
    float TransparencyScalar;
    float3 AmbientColor;
    float DensityScalar;
    float3 SpecularColor;
    float SpecularScalar;
    float3 EmissiveColor;
    uint TextureFlag;
};

cbuffer MaterialConstants : register(b4)
{
    FMaterial Material;
}

cbuffer LightingConstants : register(b5)
{
     // FireBall 정보
    int FireBallCount;          // 활성화된 FireBall 개수
    float3 Padding0;            // 16바이트 정렬용 패딩

    struct FireBallInfo
    {
        float3 Position; // FireBall 위치
        float Radius; // 반경
        float4 Color; // RGB 색상 + Alpha
        float Intensity; // 강도
        float RadiusFallOff; // 감쇠 계수
        float2 Padding; // 16바이트 정렬용 패딩
    } FireBalls[8];
};

cbuffer SubMeshConstants : register(b6)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPadding;
}

cbuffer TextureConstants : register(b7)
{
    float2 UVOffset;
    float2 TexturePadding;
}

struct PS_INPUT
{
    float4 position : SV_POSITION; // 변환된 화면 좌표
    float4 color : COLOR; // 전달할 색상
    float3 normal : NORMAL; // 정규화된 노멀 벡터
    float2 texcoord : TEXCOORD1;
    int materialIndex : MATERIAL_INDEX;
    float3 worldPos : TEXCOORD2; // 월드 공간 좌표 추가
    float3 CameraPos : TEXCOORD3;
    float2 Velocity : TEXCOORD4;
    float3 ViewNormal : TEXCOORD5;
};

struct PS_OUTPUT
{
    float4 color : SV_Target0;
    float4 UUID : SV_Target1;
    float4 WorldPos : SV_Target2;
    float4 Velocity : SV_Target3;
    float4 Normal : SV_Target4;
};

float noise(float3 p)
{
    return frac(sin(dot(p, float3(12.9898, 78.233, 37.719))) * 43758.5453);
}

float LinearToSRGB(float val)
{
    float low  = 12.92 * val;
    float high = 1.055 * pow(val, 1.0 / 2.4) - 0.055;
    // linear가 임계값보다 큰지 판별 후 선형 보간
    float t = step(0.0031308, val); // linear >= 0.0031308이면 t = 1, 아니면 t = 0
    return lerp(low, high, t);
}

float SRGBToLinear(float val)
{
    float low  = val / 12.92;
    float high = pow((val + 0.055) / 1.055, 2.4);
    float t = step(0.04045, val); // srgb가 0.04045 이상이면 t = 1, 아니면 t = 0
    return lerp(low, high, t);
}

float4 PaperTexture(float3 originalColor)
{
    // 입력 색상을 0~1 범위로 제한
    float3 color = saturate(originalColor);

    float3 paperColor = float3(0.95, 0.95, 0.95);
    float blendFactor = 0.5;
    float3 mixedColor = lerp(color, paperColor, blendFactor);

    // 정적 grain 효과
    float grain = noise(color * 10.0) * 0.1;

    // 거친 질감 효과: 두 단계의 노이즈 레이어를 결합
    float rough1 = (noise(color * 20.0) - 0.5) * 0.15; // 첫 번째 레이어: 큰 규모의 노이즈
    float rough2 = (noise(color * 40.0) - 0.5) * 0.01; // 두 번째 레이어: 세부 질감 추가
    float rough = rough1 + rough2;

    // vignette 효과: 중앙에서 멀어질수록 어두워지는 효과
    float vignetting = smoothstep(0.4, 1.0, length(color.xy - 0.5) * 2.0);

    // 최종 색상 계산
    float3 finalColor = mixedColor + grain + rough - vignetting * 0.1;
    return float4(saturate(finalColor), 1.0);
}

PS_OUTPUT mainPS(PS_INPUT input)
{
    PS_OUTPUT output;

    output.UUID = UUID;
    output.Velocity = float4(input.Velocity, 0, 1);
    //output.WorldPos = float4(input.WorldPos * 0.5 + 0.5, 0, 1);
    float3 NormalVS = normalize(input.ViewNormal);
    output.Normal = float4(NormalVS * 0.5f + 0.5f, 1.0f); // 저장용 (0~1로 remap)

    float3 color = Material.DiffuseColor;
    if (Material.TextureFlag & (1 << 0))
    {
        color = DiffuseTexture.Sample(Sampler, input.texcoord + UVOffset);
    }

    float3 EmissiveColor = Material.EmissiveColor;
    if (Material.TextureFlag & (1 << 1))
    {
        EmissiveColor = EmissiveTexture.Sample(Sampler, input.texcoord + UVOffset);
    }

    float3 SpecularColor = Material.SpecularColor;

    float Shininess = Material.SpecularScalar;
    if (Material.TextureFlag & (1 << 3))
    {
        float3 RoughnessMap = RoughnessTexture.Sample(Sampler, input.texcoord + UVOffset);
        Shininess = 1.f - saturate(max(max(RoughnessMap.r, RoughnessMap.g), RoughnessMap.b));
    }

    if (IsSelected)
    {
       // color += float3(0.2f, 0.2f, 0.0f); // 노란색 틴트로 하이라이트
        //if (IsSelectedSubMesh)
         //   color = float3(1, 1, 1);
    }

    // 발광 색상 추가

    if (IsLit == 1) // 조명이 적용되는 경우
    {
        float3 Normal = normalize(input.normal);

        // Begin Diffuse
        float3 LightDirection = normalize(float3(-1.0f, -0.3f, -2.0f));
        float Lambert = max(dot(Normal, -LightDirection), 0.0f);
        float3 Diffuse = Lambert * color;
        // End Diffuse

        // Begin Specular
        float3 ViewDirection = normalize(input.CameraPos - input.worldPos);
        float3 ReflectedDirection = reflect(LightDirection, Normal);
        float SpecAngle = max(dot(ViewDirection, ReflectedDirection), 0.0f);
        float3 Specular = pow(SpecAngle, Shininess) * SpecularColor;
        // End Specular

        float3 FinalColor = (Diffuse + Specular) * 0.1f;

        // FireBall 조명 계산 (원본 코드 유지)
        for (int i = 0; i < FireBallCount; i++)
        {
            // 반경이 0인 FireBall은 건너뛰기 (비활성화된 슬롯)
            if (FireBalls[i].Radius <= 0.0f)
            {
                continue;
            }

            float3 PointLightVector = FireBalls[i].Position - input.worldPos;
            float Distance = length(PointLightVector);
            if (Distance >= FireBalls[i].Radius)
            {
                continue;
            }

            float3 PointLightDirection = normalize(PointLightVector);
            float normalizedDist = Distance / FireBalls[i].Radius; // [0 ~ 1]
            float attenuation = 1.0 - pow(normalizedDist, FireBalls[i].RadiusFallOff);

            // Diffuse 계산
            float PointLambert = max(dot(Normal, PointLightDirection), 0.0f);

            // Specular 계산
            float3 PointReflected = reflect(-PointLightDirection, Normal);
            float PointSpecAngle = max(dot(ViewDirection, PointReflected), 0.0f);
            float3 PointSpecular = pow(PointSpecAngle, Shininess) * SpecularColor;

            // 포인트 라이트 색상과 강도 적용
            float3 FireBallColor = FireBalls[i].Color.rgb * FireBalls[i].Intensity;
            float3 PointDiffuse = PointLambert * FireBallColor;

            // 두 조명 성분 누적 (attenuation 적용)
            FinalColor += attenuation * (PointDiffuse + PointSpecular) * color;
        }

        // 색상 클램핑 (HDR 효과를 원한다면 이 부분을 조정)
        float3 Ambient = color * 0.01f; // Ambient 확 줄임
        FinalColor = saturate(FinalColor) + Ambient + EmissiveColor * 4.f;

        output.color = float4(FinalColor, Material.TransparencyScalar);
        return output;
    }
    else // unlit 상태일 때 PaperTexture 효과 적용
    {
        output.color = float4(color, 1);
        // 투명도 적용
        output.color.a = Material.TransparencyScalar;

        return output;
    }
}