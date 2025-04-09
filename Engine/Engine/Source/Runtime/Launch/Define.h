#pragma once
#include <cmath>
#include <algorithm>
#include "Core/Container/String.h"
#include "Core/Container/Array.h"
#include "UObject/NameTypes.h"

// 수학 관련
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"


#define UE_LOG Console::GetInstance().AddLog

#define _TCHAR_DEFINED
#include <d3d11.h>

#include "UserInterface/Console.h"

class UWorld;

struct FStaticMeshVertex
{
    float X, Y, Z;    // Position
    float NormalX, NormalY, NormalZ;
    float TangentX, TangentY, TangentZ;
    float U = 0, V = 0;
    float R, G, B, A; // Color
    uint32 MaterialIndex;
};

// Material Subset
struct FMaterialSubset
{
    uint32 IndexStart; // Index Buffer Start pos
    uint32 IndexCount; // Index Count
    uint32 MaterialIndex; // Material Index
    FString MaterialName; // Material Name
};

struct FStaticMaterial
{
    class UMaterial* Material;
    FName MaterialSlotName;
    //FMeshUVChannelInfo UVChannelData;
};

// OBJ File Raw Data
struct FObjInfo
{
    FWString ObjectName; // OBJ File Name
    FWString PathName; // OBJ File Paths
    FString DisplayName; // Display Name
    FString MatName; // OBJ MTL File Name

    // Group
    uint32 NumOfGroup = 0; // token 'g' or 'o'
    TArray<FString> GroupName;

    // Vertex, UV, Normal List
    TArray<FVector> Vertices;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;

    // Faces
    TArray<int32> Faces;

    // Index
    TArray<uint32> VertexIndices;
    TArray<uint32> NormalIndices;
    TArray<uint32> UVIndices;

    // Material
    TArray<FMaterialSubset> MaterialSubsets;
};

struct FObjMaterialInfo
{
    FString MaterialName;  // newmtl : Material Name.

    bool bHasTexture = false;  // Has Texture?
    bool bTransparent = false; // Has alpha channel?

    uint32 TextureFlag = 0;

    FVector Diffuse;  // Kd : Diffuse (Vector4)
    FVector Specular;  // Ks : Specular (Vector)
    FVector Ambient;   // Ka : Ambient (Vector)
    FVector Emissive;  // Ke : Emissive (Vector)

    float SpecularScalar; // Ns : Specular Power (Float)
    float DensityScalar;  // Ni : Optical Density (Float)
    float TransparencyScalar; // d or Tr  : Transparency of surface (Float)

    uint32 IlluminanceModel; // illum: illumination Model between 0 and 10. (UINT)

    /* Texture */
    FString DiffuseTextureName;  // map_Kd : Diffuse texture
    FWString DiffuseTexturePath;

    FString AmbientTextureName;  // map_Ka : Ambient texture
    FWString AmbientTexturePath;

    FString SpecularTextureName; // map_Ks : Specular Color texture
    FWString SpecularTexturePath;

    FString BumpTextureName;     // map_Bump : Bump texture
    FWString BumpTexturePath;

    FString AlphaTextureName;    // map_d : Alpha texture
    FWString AlphaTexturePath;

    FString EmissiveTextureName;    // map_Ke : Emissive texture
    FWString EmissiveTexturePath;

    FString RoughnessTextureName;    // map_Ns : Roughness texture
    FWString RoughnessTexturePath;

    FString NormalTextureName;    // map_Bump : Roughness texture
    FWString NormalTexturePath;
};

enum class EWorldType
{
    Editor,
    EditorPreview,
    PIE,
    Game,
};

struct FWorldContext
{
    UWorld* World;
    EWorldType worldType;

};

// Cooked Data
namespace OBJ
{
    struct FStaticMeshRenderData
    {
        FWString ObjectName;
        FWString PathName;
        FString DisplayName;

        TArray<FStaticMeshVertex> Vertices;
        TArray<UINT> Indices;

        ID3D11Buffer* VertexBuffer;
        ID3D11Buffer* IndexBuffer;

        TArray<FObjMaterialInfo> Materials;
        TArray<FMaterialSubset> MaterialSubsets;

        FVector BoundingBoxMin;
        FVector BoundingBoxMax;
    };
}

struct FVertexTexture
{
	float x, y, z;    // Position
	float u, v; // Texture
};

struct FGridParameters
{
	float gridSpacing;
	int   numGridLines;
	FVector gridOrigin;
	float pad;
};

struct FSimpleVertex
{
	float dummy; // 내용은 사용되지 않음
    float padding[11];
};

struct FOrientedBoundingBox
{
    FVector corners[8];
};

struct FRect
{
    FRect() : leftTopX(0), leftTopY(0), width(0), height(0) {}
    FRect(float x, float y, float w, float h) : leftTopX(x), leftTopY(y), width(w), height(h) {}
    float leftTopX, leftTopY, width, height;
};

struct FPoint
{
    FPoint() : x(0), y(0) {}
    FPoint(float _x, float _y) : x(_x), y(_y) {}
    FPoint(long _x, long _y) : x(_x), y(_y) {}
    FPoint(int _x, int _y) : x(_x), y(_y) {}

    float x, y;
};

struct FBoundingBox
{
    FBoundingBox() {}
    FBoundingBox(FVector _min, FVector _max) : min(_min), max(_max) {}

	FVector min; // Minimum extents
	float pad;
	FVector max; // Maximum extents
	float pad1;

    bool Intersect(const FVector& rayOrigin, const FVector& rayDir, float& outDistance)
    {
        float tmin = -FLT_MAX;
        float tmax = FLT_MAX;
        const float epsilon = 1e-6f;

        // X축 처리
        if (fabs(rayDir.X) < epsilon)
        {
            // 레이가 X축 방향으로 거의 평행한 경우,
            // 원점의 x가 박스 [min.x, max.x] 범위 밖이면 교차 없음
            if (rayOrigin.X < min.X || rayOrigin.X > max.X)
                return false;
        }
        else
        {
            float t1 = (min.X - rayOrigin.X) / rayDir.X;
            float t2 = (max.X - rayOrigin.X) / rayDir.X;
            if (t1 > t2)  std::swap(t1, t2);

            // tmin은 "현재까지의 교차 구간 중 가장 큰 min"
            tmin = (t1 > tmin) ? t1 : tmin;
            // tmax는 "현재까지의 교차 구간 중 가장 작은 max"
            tmax = (t2 < tmax) ? t2 : tmax;
            if (tmin > tmax)
                return false;
        }

        // Y축 처리
        if (fabs(rayDir.Y) < epsilon)
        {
            if (rayOrigin.Y < min.Y || rayOrigin.Y > max.Y)
                return false;
        }
        else
        {
            float t1 = (min.Y - rayOrigin.Y) / rayDir.Y;
            float t2 = (max.Y - rayOrigin.Y) / rayDir.Y;
            if (t1 > t2)  std::swap(t1, t2);

            tmin = (t1 > tmin) ? t1 : tmin;
            tmax = (t2 < tmax) ? t2 : tmax;
            if (tmin > tmax)
                return false;
        }

        // Z축 처리
        if (fabs(rayDir.Z) < epsilon)
        {
            if (rayOrigin.Z < min.Z || rayOrigin.Z > max.Z)
                return false;
        }
        else
        {
            float t1 = (min.Z - rayOrigin.Z) / rayDir.Z;
            float t2 = (max.Z - rayOrigin.Z) / rayDir.Z;
            if (t1 > t2)  std::swap(t1, t2);

            tmin = (t1 > tmin) ? t1 : tmin;
            tmax = (t2 < tmax) ? t2 : tmax;
            if (tmin > tmax)
                return false;
        }

        // 여기까지 왔으면 교차 구간 [tmin, tmax]가 유효하다.
        // tmax < 0 이면, 레이가 박스 뒤쪽에서 교차하므로 화면상 보기엔 교차 안 한다고 볼 수 있음
        if (tmax < 0.0f)
            return false;

        // outDistance = tmin이 0보다 크면 그게 레이가 처음으로 박스를 만나는 지점
        // 만약 tmin < 0 이면, 레이의 시작점이 박스 내부에 있다는 의미이므로, 거리를 0으로 처리해도 됨.
        outDistance = (tmin >= 0.0f) ? tmin : 0.0f;

        return true;
    }

};

struct FCone
{
    FVector ConeApex; // 원뿔의 꼭짓점
    float ConeRadius; // 원뿔 밑면 반지름

    FVector ConeBaseCenter; // 원뿔 밑면 중심
    float ConeHeight; // 원뿔 높이 (Apex와 BaseCenter 간 차이)
    FVector4 Color;

    int ConeSegmentCount; // 원뿔 밑면 분할 수
    float pad[3];

};

struct FPrimitiveCounts
{
	int BoundingBoxCount;
	int ConeCount;
	int pad;
	int pad1;
};

struct FFireBallData
{
    FVector Position;     // FireBall 위치
    float Radius;         // 반경
    FVector4 Color;       // RGB 색상 + Alpha
    float Intensity;      // 강도
    float RadiusFallOff;  // 감쇠 계수
    float Padding[2];     // 16바이트 정렬
};
struct FLighting
{
    static const int MAX_FIREBALLS = 8; // 최대 지원 FireBall 수

    int FireBallCount;    // 활성화된 FireBall 개수
    float Padding[3];     // 16바이트 정렬

    FFireBallData FireBalls[MAX_FIREBALLS]; // FireBall 배열
};

struct FMaterialConstants
{
    FVector DiffuseColor;
    float TransparencyScalar;
    FVector AmbientColor;
    float DensityScalar;
    FVector SpecularColor;
    float SpecularScalar;
    FVector EmmisiveColor;
    uint32 TextureFlag;
};

struct FObjectConstants
{
    FMatrix ModelMatrix;      // 모델
    FMatrix ModelMatrixInverseTranspose; // normal 변환을 위한 행렬
    FMatrix PrevModelMatrix;
    FVector4 UUIDColor;
    bool IsSelected;
    FVector pad;
    FMatrix M;
};

struct FViewConstants
{
    FMatrix ViewMatrix;
    FMatrix InvViewMatrix;
    FMatrix PrevViewMatrix;
    FVector ViewLocation;
    float ViewPadding;
};

struct FProjectionConstants
{
    FMatrix ProjectionMatrix;
    FMatrix InvProjectionMatrix;
    FMatrix PrevProjectionMatrix;
    float NearClip;
    float FarClip;
    FVector2D ProjectionPadding;
};

struct FLitUnlitConstants
{
    int isLit; // 1 = Lit, 0 = Unlit
    FVector pad;
};

struct FSubMeshConstants
{
    float isSelectedSubMesh;
    FVector pad;
};

struct FTextureConstants
{
    float UOffset;
    float VOffset;
    float pad0;
    float pad1;
};

struct alignas(16) FExponentialHeightFogConstants
{
    FVector FogColor;
    float FogDensity;
    float FogFalloff;
    float FogHeight;

    float FogStartDistance;     // 안개 시작 거리
    float FogEndDistance;       // 안개 끝 거리
    float DistanceFogIntensity; // 거리 기반 안개 영향
    FVector Padding123;
};

struct alignas(16) FMotionBlurConstants
{
    float ScreenSizeX;       // 화면 크기
    float ScreenSizeY;       // 화면 크기
    float  MaxBlurPixels;    // 최대 블러 거리 (픽셀 단위)
    float  VelocityScale;
    float  DepthThreshold;
    FVector padding;
};