#include "BillboardComponent.h"
#include "Actors/Player.h"
#include "QuadTexture.h"
#include "Define.h"
#include <DirectXMath.h>

#include "Level.h"
#include "Math/MathUtility.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"
#include "PropertyEditor/ShowFlags.h"

UBillboardComponent::UBillboardComponent()
{
    SetType(StaticClass()->GetName());
}

UBillboardComponent::~UBillboardComponent()
{
	if (vertexTextureBuffer)
	{
		vertexTextureBuffer->Release();
		vertexTextureBuffer = nullptr;
	}
	if (indexTextureBuffer)
	{
		indexTextureBuffer->Release();
		indexTextureBuffer = nullptr;
	}
}

void UBillboardComponent::DuplicateSubObjects()
{
    // deepcopy 대상 없음.
}

UObject* UBillboardComponent::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UBillboardComponent>(this);

    Cast<UBillboardComponent>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void UBillboardComponent::InitializeComponent()
{
    Super::InitializeComponent();

	CreateQuadTextureVertexBuffer();
}

void UBillboardComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}


int UBillboardComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
	TArray<FVector> quad;
	for (int i = 0; i < 4; i++)
	{
		quad.Add(FVector(quadTextureVertices[i].x, quadTextureVertices[i].y, quadTextureVertices[i].z));
	}
	return CheckPickingOnNDC(quad,pfNearHitDistance);
}


void UBillboardComponent::SetTexture(FWString _fileName)
{
	Texture = FEngineLoop::ResourceManager.GetTexture(_fileName);
}

FMatrix UBillboardComponent::CreateBillboardMatrix()
{
	FMatrix CameraView = GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();

	CameraView.M[0][3] = 0.0f;
	CameraView.M[1][3] = 0.0f;
	CameraView.M[2][3] = 0.0f;


	CameraView.M[3][0] = 0.0f;
	CameraView.M[3][1] = 0.0f;
	CameraView.M[3][2] = 0.0f;
	CameraView.M[3][3] = 1.0f;


	CameraView.M[0][2] = -CameraView.M[0][2];
	CameraView.M[1][2] = -CameraView.M[1][2];
	CameraView.M[2][2] = -CameraView.M[2][2];
	FMatrix LookAtCamera = FMatrix::Transpose(CameraView);

	FVector worldLocation = GetWorldLocation();
	FVector worldScale = RelativeScale3D;
	FMatrix S = FMatrix::CreateScale(worldScale.X, worldScale.Y, worldScale.Z);
	FMatrix R = LookAtCamera;
	FMatrix T = FMatrix::CreateTranslationMatrix(worldLocation);
	FMatrix M = S * R * T;

	return M;
}

void UBillboardComponent::CreateQuadTextureVertexBuffer()
{
	numVertices = sizeof(quadTextureVertices) / sizeof(FVertexTexture);
	numIndices = sizeof(quadTextureInices) / sizeof(uint32);
	vertexTextureBuffer = FEngineLoop::Renderer.CreateVertexBuffer(quadTextureVertices, sizeof(quadTextureVertices));
	indexTextureBuffer = FEngineLoop::Renderer.CreateIndexBuffer(quadTextureInices, sizeof(quadTextureInices));

	if (!vertexTextureBuffer)
	{
		Console::GetInstance().AddLog(LogLevel::Warning, "Buffer Error");
	}
	if (!indexTextureBuffer)
	{
		Console::GetInstance().AddLog(LogLevel::Warning, "Buffer Error");
	}
}

bool UBillboardComponent::CheckPickingOnNDC(const TArray<FVector>& checkQuad, float& hitDistance)
{
	bool result = false;
	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(GEngineLoop.hWnd, &mousePos);

	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	FEngineLoop::GraphicDevice.DeviceContext->RSGetViewports(&numViewports, &viewport);
	float screenWidth = viewport.Width;
	float screenHeight = viewport.Height;

	FVector pickPosition;
	int screenX = mousePos.x;
	int screenY = mousePos.y;
    FMatrix projectionMatrix = GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetProjectionMatrix();
	pickPosition.X = ((2.0f * screenX / viewport.Width) - 1);
	pickPosition.Y = -((2.0f * screenY / viewport.Height) - 1);
	pickPosition.Z = 1.0f; // Near Plane

	FMatrix M = CreateBillboardMatrix();
    FMatrix V = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();;
	FMatrix P = projectionMatrix;
	FMatrix MVP = M * V * P;

	float minX = FLT_MAX;
	float maxX = FLT_MIN;
	float minY = FLT_MAX;
	float maxY = FLT_MIN;
	float avgZ = 0.0f;
	for (int i = 0; i < checkQuad.Num(); i++)
	{
		FVector4 v = FVector4(checkQuad[i].X, checkQuad[i].Y, checkQuad[i].Z, 1.0f);
		FVector4 clipPos = FMatrix::TransformVector(v, MVP);

		if (clipPos.a != 0)	clipPos = clipPos/clipPos.a;

		minX = FMath::Min(minX, clipPos.x);
		maxX = FMath::Max(maxX, clipPos.x);
		minY = FMath::Min(minY, clipPos.y);
		maxY = FMath::Max(maxY, clipPos.y);
		avgZ += clipPos.z;
	}

	avgZ /= checkQuad.Num();

	if (pickPosition.X >= minX && pickPosition.X <= maxX &&
		pickPosition.Y >= minY && pickPosition.Y <= maxY)
	{
		float A = P.M[2][2];  // Projection Matrix의 A값 (Z 변환 계수)
		float B = P.M[3][2];  // Projection Matrix의 B값 (Z 변환 계수)

		float z_view_pick = (pickPosition.Z - B) / A; // 마우스 클릭 View 공간 Z
		float z_view_billboard = (avgZ - B) / A; // Billboard View 공간 Z

		hitDistance = 1000.0f;
		result = true;
	}

	return result;
}
