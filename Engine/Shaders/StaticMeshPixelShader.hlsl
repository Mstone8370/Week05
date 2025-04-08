#include "ShaderRegisters.hlsl"

Texture2D Textures : register(t0);

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
    float MaterialPad0;
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
    bool normalFlag : TEXCOORD0; // 노멀 유효성 플래그 (1.0: 유효, 0.0: 무효)
    float2 texcoord : TEXCOORD1;
    int materialIndex : MATERIAL_INDEX;
    float3 worldPos : TEXCOORD2; // 월드 공간 좌표 추가
    float3 cameraPos : TEXCOORD3;
};

struct PS_OUTPUT
{
    float4 color : SV_Target0;
    float4 UUID : SV_Target1;
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

    float3 texColor = Textures.Sample(Sampler, input.texcoord + UVOffset);
    float3 color;
    if (texColor.g == 0) // TODO: boolean으로 변경
        color = saturate(Material.DiffuseColor);
    else
    {
        color = texColor + Material.DiffuseColor;
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
        
        if (input.normalFlag < 0.5)
        {
            output.color = float4(color, Material.TransparencyScalar);
            return output;
        }
        
        float3 N = normalize(input.normal);
        
        float3 lightDir = normalize(float3(-1.0f, -1.0f, -1.0f));
        float diff = max(dot(N, -lightDir), 0.0f);

// 그림자 대비 강화
        diff = pow(diff, 1.5f); // 약한 각도는 더 어둡게, 정면은 밝게

        float3 diffuse = diff * color * 1.0f; // 조명 세기 강화
        float3 ambient = color * 0.05f; // Ambient 확 줄임

        float3 finalColor = diffuse;


        
        // FireBall 조명 계산 (원본 코드 유지)
        for (int i = 0; i < FireBallCount; i++)
        {
            // 반경이 0인 FireBall은 건너뛰기 (비활성화된 슬롯)
            if (FireBalls[i].Radius <= 0.0f)
                continue;
        
            // FireBall과의 거리 계산
            float3 lightVec = FireBalls[i].Position - input.worldPos;
            float distance = length(lightVec);
            // 반경 내에 있는 경우에만 조명 적용
            if (distance < FireBalls[i].Radius)
            {
                // 정규화된 거리 (0~1)
                float normalizedDist = distance / FireBalls[i].Radius;
            
                // 감쇠 계산
                float attenuation = 1.0 - pow(normalizedDist, FireBalls[i].RadiusFallOff);
            
                // 표면 각도 계산
                float3 L = normalize(lightVec);
                float NdotL = max(0.0f, dot(N, L));
            
                // FireBall 조명 기여도 계산
                float3 fireBallLight = FireBalls[i].Color.rgb * attenuation * NdotL * FireBalls[i].Intensity;
            
                // 최종 색상에 FireBall 조명 추가
                finalColor += fireBallLight * color;
            }
        }
    
        // 색상 클램핑 (HDR 효과를 원한다면 이 부분을 조정)
        finalColor = saturate(finalColor);
    
        output.color = float4(finalColor, Material.TransparencyScalar);
        return output;
       
    }
    else // unlit 상태일 때 PaperTexture 효과 적용
    {
        if (input.normalFlag < 0.5)
        {
            output.color = float4(color, Material.TransparencyScalar);
            return output;
        }

        output.color = float4(color, 1);
        // 투명도 적용
        output.color.a = Material.TransparencyScalar;

        return output;
    }
}