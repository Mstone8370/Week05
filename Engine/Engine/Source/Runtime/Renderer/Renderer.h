#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>
#include "EngineBaseTypes.h"
#include "Define.h"
#include "Container/Set.h"

class UMotionBlurComponent;
class UExponentialHeightFogComponent;
class UPrimitiveComponent;
class ULightComponentBase;
class ULevel;
class FGraphicsDevice;
class UMaterial;
class UObject;
class FEditorViewportClient;
class UBillboardComponent;
class UStaticMeshComponent;
class UGizmoBaseComponent;
struct FStaticMaterial;
class UPointLightComponent;


class FRenderer
{

private:
    uint16 ViewModeFlags = 0;

public:
    FGraphicsDevice* Graphics;
    ID3D11VertexShader* VertexShader = nullptr;
    ID3D11PixelShader* PixelShader = nullptr;
    ID3D11InputLayout* InputLayout = nullptr;

    ID3D11VertexShader* FinalVertexShader = nullptr;
    ID3D11PixelShader* FinalPixelShader = nullptr;
    ID3D11PixelShader* NormalizedDepthShader = nullptr;
    ID3D11PixelShader* VisualizationVelocityShader = nullptr;

    // Post Process
    ID3D11PixelShader* FogShader = nullptr;
    ID3D11PixelShader* MotionBlurShader = nullptr;

    ID3D11Buffer* ObjectConstantBuffer = nullptr;
    ID3D11Buffer* ViewConstantBuffer = nullptr;
    ID3D11Buffer* ProjectionConstantBuffer = nullptr;

    ID3D11Buffer* LightingBuffer = nullptr;
    ID3D11Buffer* FlagBuffer = nullptr;
    ID3D11Buffer* MaterialConstantBuffer = nullptr;
    ID3D11Buffer* SubMeshConstantBuffer = nullptr;
    ID3D11Buffer* TextureConstantBuffer = nullptr;
    ID3D11Buffer* ExponentialConstantBuffer = nullptr;
    ID3D11Buffer* MotionBlurConstantBuffer = nullptr;

    ID3D11SamplerState* ScreenSamplerState = nullptr;

    FLighting lightingData;

    void Initialize(FGraphicsDevice* graphics);

    void PrepareShader() const;

    //Render
    void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const;
    void RenderPrimitive(ID3D11Buffer* pVertexBuffer, ID3D11Buffer* pIndexBuffer, UINT numIndices) const;
    void RenderPrimitive(OBJ::FStaticMeshRenderData* RenderData, TArray<FStaticMaterial*> Materials, TArray<UMaterial*> OverrideMaterial, int SelectedSubMeshIndex) const;

    void RenderTexturedModelPrimitive(ID3D11Buffer* pVertexBuffer, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* InTextureSRV, ID3D11SamplerState* InSamplerState) const;
    //Release
    void Release();
    void ReleaseShader();
    static void ReleaseShader(ID3D11VertexShader*& Shader);
    static void ReleaseShader(ID3D11PixelShader*& Shader);
    static void ReleaseBuffer(ID3D11Buffer*& Buffer);
    void ReleaseConstantBuffer();

    void ResetVertexShader();
    void ResetPixelShader();


private:
    void CreateShader();

    void CreateFinalShader();
    void ReleaseFinalShader();

    // Visualization
    void CreateNormalizedDepthShader();
    void CreateVisualizationVelocityShader();

    // PostProcess
    void CreateFogShader();
    void CreateMotionBlurShader();

    void CreateVisualizationShader();
    void CreatePostProcessShader();
    void ReleaseVisualizationShader();
    void ReleasePostProcessShader();

    void CreateScreenSamplerState();
    void ReleaseScreenSamplerState();

    // CreateBuffer
    void CreateConstantBuffer();
    void CreateLightingBuffer();
    void CreateLitUnlitBuffer();

public:
    ID3D11Buffer* CreateVertexBuffer(FStaticMeshVertex* vertices, UINT byteWidth) const;
    ID3D11Buffer* CreateVertexBuffer(const TArray<FStaticMeshVertex>& vertices, UINT byteWidth) const;
    ID3D11Buffer* CreateIndexBuffer(uint32* Indices, UINT ByteWidth) const;
    ID3D11Buffer* CreateIndexBuffer(const TArray<uint32>& Indices, UINT ByteWidth) const;

public:
    void ChangeViewMode(EViewModeIndex InViewModeIndex);
    // update
    void UpdateObjectBuffer(const FMatrix& PrevModelMatrix, const FMatrix& ModelMatrix, const FMatrix& InverseTransposedNormal, FVector4 UUIDColor, bool
                            IsSelected) const;
    void UpdateViewBuffer(const FMatrix& PrevViewMatrix, const FMatrix& ViewMatrix, const FVector& ViewLocation) const;
    void UpdateProjectionBuffer(const FMatrix& PrevProjectionMatrix, const FMatrix& ProjectionMatrix, float NearClip, float FarClip) const;

    void UpdateLightBuffer() const;
    void UpdateMaterial(const FObjMaterialInfo& MaterialInfo) const;
    void UpdateLitUnlitConstant(int isLit) const;
    void UpdateSubMeshConstant(bool isSelected) const;
    void UpdateTextureConstant(float UOffset, float VOffset);

    void UpdateExponentialHeightFogConstant(UExponentialHeightFogComponent* ExponentialHeightFogComp);
    void UpdateMotionBlurConstant(UMotionBlurComponent* MotionBlurComponent, std::shared_ptr<FEditorViewportClient> ActiveViewport);

    //텍스쳐용 기능 추가
    ID3D11VertexShader* TextureVertexShader = nullptr;
    ID3D11PixelShader* TexturePixelShader = nullptr;
    ID3D11InputLayout* TextureInputLayout = nullptr;

    ID3D11VertexShader* FontVertexShader = nullptr;
    ID3D11PixelShader* FontPixelShader = nullptr;
    ID3D11InputLayout* FontInputLayout = nullptr;

    struct FSubUVConstant
    {
        float indexU;
        float indexV;
    };
    ID3D11Buffer* SubUVConstantBuffer = nullptr;

    void CreateFontShader();
    void ReleaseFontShader();

    void PrepareFontShader() const;

    void CreateTextureShader();
    void ReleaseTextureShader();

    void PrepareTextureShader() const;
    ID3D11Buffer* CreateVertexTextureBuffer(FVertexTexture* vertices, UINT byteWidth) const;
    ID3D11Buffer* CreateIndexTextureBuffer(uint32* indices, UINT byteWidth) const;
    void RenderTexturePrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
        ID3D11Buffer* pIndexBuffer, UINT numIndices,
        ID3D11ShaderResourceView* _TextureSRV,
        ID3D11SamplerState* _SamplerState) const;
    void RenderTextPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
        ID3D11ShaderResourceView* _TextureSRV,
        ID3D11SamplerState* _SamplerState) const;
    ID3D11Buffer* CreateVertexBuffer(FVertexTexture* vertices, UINT byteWidth) const;

    void UpdateSubUVConstant(float _indexU, float _indexV) const;
    void PrepareSubUVConstant() const;
    void SetDefaultConstantBuffer() const;
    void PrepareRender(ULevel* Level);
    void RenderScene(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport);

    void SampleAndProcessSRV(std::shared_ptr<FEditorViewportClient> ActiveViewport, uint32 ViewportIndex);

    void PreparePostProcess(std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void PostProcess(std::shared_ptr<FEditorViewportClient> ActiveViewport, uint32 ViewportIndex);

    void RenderFullScreenQuad(std::shared_ptr<FEditorViewportClient> ActiveViewport, uint32 ViewportIndex);

    void DrawFullScreenQuad();

    // line shader
    void PrepareLineShader() const;
    void CreateLineShader();
    void ReleaseLineShader();
    void RenderBatch(const FGridParameters& gridParam, ID3D11Buffer* pVertexBuffer, int boundingBoxCount, int coneCount, int coneSegmentCount, int obbCount) const;
    void UpdateGridConstantBuffer(const FGridParameters& gridParams) const;
    void UpdateLinePrimitveCountBuffer(int numBoundingBoxes, int numCones) const;
    ID3D11Buffer* CreateStaticVerticesBuffer() const;
    ID3D11Buffer* CreateBoundingBoxBuffer(UINT numBoundingBoxes) const;
    ID3D11Buffer* CreateOBBBuffer(UINT numBoundingBoxes) const;
    ID3D11Buffer* CreateConeBuffer(UINT numCones) const;
    ID3D11ShaderResourceView* CreateBoundingBoxSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes);
    ID3D11ShaderResourceView* CreateOBBSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes);
    ID3D11ShaderResourceView* CreateConeSRV(ID3D11Buffer* pConeBuffer, UINT numCones);

    void UpdateBoundingBoxBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FBoundingBox>& BoundingBoxes, int numBoundingBoxes) const;
    void UpdateOBBBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FOrientedBoundingBox>& BoundingBoxes, int numBoundingBoxes) const;
    void UpdateConesBuffer(ID3D11Buffer* pConeBuffer, const TArray<FCone>& Cones, int numCones) const;

    void OnEndRender();
private:
    void ClearRenderArr();
    void RenderStaticMeshes(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderGizmos(const ULevel* Level, const std::shared_ptr<FEditorViewportClient>& ActiveViewport);
    void RenderLight(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderBillboards(ULevel* Level,std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderTexts(ULevel* Level,std::shared_ptr<FEditorViewportClient> ActiveViewport);

private:
    TArray<UStaticMeshComponent*> StaticMeshObjs;
    TArray<UGizmoBaseComponent*> GizmoObjs;
    TArray<UPrimitiveComponent*> TextObjs;
    TArray<UBillboardComponent*> BillboardObjs;
    TArray<ULightComponentBase*> LightObjs;
    TArray<UPointLightComponent*> FireBalls;

    float litFlag = 0;

    // TODO: Post Processing Target 따로 정리
    UExponentialHeightFogComponent* ExponentialHeightFogComponent = nullptr;

    // TODO: 임시로 아래 추가, 포스트 프로세싱 구조 변경 시 리팩토링 필요.
    UMotionBlurComponent* MotionBlurComponent = nullptr;

public:
    ID3D11VertexShader* VertexLineShader = nullptr;
    ID3D11PixelShader* PixelLineShader = nullptr;
    ID3D11Buffer* GridConstantBuffer = nullptr;
    ID3D11Buffer* LinePrimitiveBuffer = nullptr;
    ID3D11ShaderResourceView* pBBSRV = nullptr;
    ID3D11ShaderResourceView* pConeSRV = nullptr;
    ID3D11ShaderResourceView* pOBBSRV = nullptr;
};

