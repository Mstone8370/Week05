#include "Engine/Source/Editor/PropertyEditor/ShowFlags.h"
#include "ParticleSubUVComp.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Level.h"
#include "LevelEditor/SLevelEditor.h"


UParticleSubUVComp::UParticleSubUVComp()
{
    SetType(StaticClass()->GetName());
    bIsLoop = true;
}

UParticleSubUVComp::~UParticleSubUVComp()
{
	if (vertexSubUVBuffer)
	{
		vertexSubUVBuffer->Release();
		vertexSubUVBuffer = nullptr;
	}
}

void UParticleSubUVComp::DuplicateSubObjects()
{
    // deepcopy 대상 없음.
    Super::DuplicateSubObjects();
}

UObject* UParticleSubUVComp::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UParticleSubUVComp>(this);

    Cast<UParticleSubUVComp>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void UParticleSubUVComp::InitializeComponent()
{
	Super::InitializeComponent();
	FEngineLoop::Renderer.UpdateSubUVConstant(0, 0);
	FEngineLoop::Renderer.PrepareSubUVConstant();
}

void UParticleSubUVComp::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    if (!IsActive()) return;

	uint32 CellWidth = Texture->width / CellsPerColumn;
	uint32 CellHeight = Texture->height / CellsPerColumn;


	second += DeltaTime;
	if (second >= 75)
	{
		indexU++;
		second = 0;
	}
	if (indexU >= CellsPerColumn)
	{
		indexU = 0;
		indexV++;
	}
	if (indexV >= CellsPerRow)
	{
		indexU = 0;
		indexV = 0;

	    // TODO: 파티클 제거는 따로 안하고, Actor에 LifeTime을 설정하든가, 파티클의 Activate 설정을 추가하던가 하기로
	    if (!bIsLoop)
	    {
            Deactivate();
	    }
	}


	float normalWidthOffset = float(CellWidth) / float(Texture->width);
	float normalHeightOffset = float(CellHeight) / float(Texture->height);

	finalIndexU = float(indexU) * normalWidthOffset;
	finalIndexV = float(indexV) * normalHeightOffset;
}

void UParticleSubUVComp::SetRowColumnCount(int _cellsPerRow, int _cellsPerColumn)
{
	CellsPerRow = _cellsPerRow;
	CellsPerColumn = _cellsPerColumn;

	CreateSubUVVertexBuffer();
}

void UParticleSubUVComp::UpdateVertexBuffer(const TArray<FVertexTexture>& vertices)
{
	/*
	ID3D11DeviceContext* context = FEngineLoop::GraphicDevice.DeviceContext;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	context->Map(vertexTextureBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, vertices.GetData(), vertices.Num() * sizeof(FVertexTexture));
	context->Unmap(vertexTextureBuffer, 0);
	*/

}

void UParticleSubUVComp::CreateSubUVVertexBuffer()
{

	uint32 CellWidth = Texture->width/CellsPerColumn;
	uint32 CellHeight = Texture->height/ CellsPerColumn;
	float normalWidthOffset = float(CellWidth) / float(Texture->width);
	float normalHeightOffset = float(CellHeight) / float(Texture->height);

	TArray<FVertexTexture> vertices =
	{
		{-1.0f,1.0f,0.0f,0,0},
		{ 1.0f,1.0f,0.0f,1,0},
		{-1.0f,-1.0f,0.0f,0,1},
		{ 1.0f,-1.0f,0.0f,1,1}
	};
	vertices[1].u = normalWidthOffset;
	vertices[2].v = normalHeightOffset;
	vertices[3].u = normalWidthOffset;
	vertices[3].v = normalHeightOffset;

	vertexSubUVBuffer = FEngineLoop::Renderer.CreateVertexBuffer(vertices.GetData(), static_cast<UINT>(vertices.Num() * sizeof(FVertexTexture)));
	numTextVertices = static_cast<UINT>(vertices.Num());
}
