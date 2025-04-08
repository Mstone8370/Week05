#include "Renderer.h"
#include <d3dcompiler.h>

#include "Level.h"
#include "Actors/Player.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/MotionBlurComponent.h"
#include "Components/ParticleSubUVComp.h"
#include "Components/TextBillboardComponent.h"
#include "Components/Material/Material.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Launch/EngineLoop.h"
#include "Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"
#include "PrimitiveBatch.h"
#include "UnrealClient.h"
#include "UObject/Casts.h"
#include "UObject/Object.h"
#include "PropertyEditor/ShowFlags.h"
#include "UObject/UObjectIterator.h"
#include "Components/SkySphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "ImGUI/imgui_internal.h"
#include "Components/UFireBallComponent.h"


void FRenderer::Initialize(FGraphicsDevice* graphics)
{
    Graphics = graphics;
    CreateShader();

    CreateVisualizationShader();
    CreatePostProcessShader();
    CreateFinalShader();


    CreateTextureShader();
    CreateFontShader();
    CreateLineShader();
    CreateConstantBuffer();
    CreateLightingBuffer();
    CreateLitUnlitBuffer();
    CreateScreenSamplerState();
    UpdateLitUnlitConstant(1);
}

void FRenderer::Release()
{
    ReleaseShader();

    ReleaseVisualizationShader();
    ReleasePostProcessShader();
    ReleaseFinalShader();

    ReleaseTextureShader();
    ReleaseFontShader();
    ReleaseLineShader();
    ReleaseConstantBuffer();
    ReleaseScreenSamplerState();
}

void FRenderer::CreateShader()
{
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    D3DCompileFromFile(L"Shaders/StaticMeshVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainVS", "vs_5_0", 0, 0, &VertexShaderCSO, nullptr);
    Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &VertexShader);

    D3DCompileFromFile(L"Shaders/StaticMeshPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainPS", "ps_5_0", 0, 0, &PixelShaderCSO, nullptr);
    Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &PixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &InputLayout
    );

    VertexShaderCSO->Release();
    PixelShaderCSO->Release();
}

void FRenderer::CreateFinalShader()
{
    ID3DBlob* FinalVertexShaderCSO;
    ID3DBlob* FinalPixelShaderCSO;

    D3DCompileFromFile(L"Shaders/FinalVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &FinalVertexShaderCSO, nullptr);
    Graphics->Device->CreateVertexShader(FinalVertexShaderCSO->GetBufferPointer(), FinalVertexShaderCSO->GetBufferSize(), nullptr, &FinalVertexShader);

    D3DCompileFromFile(L"Shaders/FinalPixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &FinalPixelShaderCSO, nullptr);
    Graphics->Device->CreatePixelShader(FinalPixelShaderCSO->GetBufferPointer(), FinalPixelShaderCSO->GetBufferSize(), nullptr, &FinalPixelShader);

    FinalVertexShaderCSO->Release();
    FinalPixelShaderCSO->Release();
}

void FRenderer::ReleaseFinalShader()
{
    ReleaseShader(FinalVertexShader);

    ReleaseShader(FinalPixelShader);
}

void FRenderer::CreateDepthShader()
{
    ID3DBlob* FinalPixelShaderCSO;

    D3DCompileFromFile(L"Shaders/NormalizeDepthShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &FinalPixelShaderCSO, nullptr);
    Graphics->Device->CreatePixelShader(FinalPixelShaderCSO->GetBufferPointer(), FinalPixelShaderCSO->GetBufferSize(), nullptr, &DepthShader);

    FinalPixelShaderCSO->Release();
}

void FRenderer::CreateFogShader()
{
    ID3DBlob* FogPixelShaderCSO;

    D3DCompileFromFile(L"Shaders/ExponentialHeightFogShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &FogPixelShaderCSO, nullptr);
    Graphics->Device->CreatePixelShader(FogPixelShaderCSO->GetBufferPointer(), FogPixelShaderCSO->GetBufferSize(), nullptr, &FogShader);

    FogPixelShaderCSO->Release();
}


void FRenderer::CreateMotionBlurShader()
{
    ID3DBlob* PixelShaderCSO;

    D3DCompileFromFile(L"Shaders/MotionBlurShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &PixelShaderCSO, nullptr);
    Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &MotionBlurShader);

    PixelShaderCSO->Release();
}

void FRenderer::CreateVisualizationShader()
{
    CreateDepthShader();
}

void FRenderer::CreatePostProcessShader()
{
    CreateMotionBlurShader();
    CreateFogShader();
}

void FRenderer::ReleaseVisualizationShader()
{
    ReleaseShader(DepthShader);
}

void FRenderer::ReleasePostProcessShader()
{
    ReleaseShader(FogShader);
    ReleaseShader(MotionBlurShader);
}

void FRenderer::CreateScreenSamplerState()
{
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    Graphics->Device->CreateSamplerState(&samplerDesc, &ScreenSamplerState);
}

void FRenderer::ReleaseScreenSamplerState()
{
    if (ScreenSamplerState)
    {
        ScreenSamplerState->Release();
        ScreenSamplerState = nullptr;
    }
}

void FRenderer::ReleaseShader()
{
    if (InputLayout)
    {
        InputLayout->Release();
        InputLayout = nullptr;
    }

    ReleaseShader(PixelShader);
    ReleaseShader(VertexShader);
}

void FRenderer::ReleaseShader(ID3D11VertexShader*& Shader)
{
    if (Shader)
    {
        Shader->Release();
        Shader = nullptr;
    }
}

void FRenderer::ReleaseShader(ID3D11PixelShader*& Shader)
{
    if (Shader)
    {
        Shader->Release();
        Shader = nullptr;
    }
}

void FRenderer::ReleaseBuffer(ID3D11Buffer*& Buffer)
{
    if (Buffer)
    {
        Buffer->Release();
        Buffer = nullptr;
    }
}

void FRenderer::PrepareShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);

    // TODO: ObjectConstantBuffer가 없어도 다른 버퍼는 Set 되도록
    if (ObjectConstantBuffer)
    {
        Graphics->DeviceContext->PSSetConstantBuffers(3, 1, &FlagBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(4, 1, &MaterialConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(5, 1, &LightingBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(6, 1, &SubMeshConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(7, 1, &TextureConstantBuffer);
    }
}

void FRenderer::ResetVertexShader()
{
    Graphics->DeviceContext->VSSetShader(nullptr, nullptr, 0);
    ReleaseShader(VertexShader);
}

void FRenderer::ResetPixelShader()
{
    Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
    ReleaseShader(PixelShader);
}

void FRenderer::ChangeViewMode(EViewModeIndex InViewModeIndex)
{
    ViewModeFlags = InViewModeIndex;
    switch (InViewModeIndex)
    {
    case EViewModeIndex::VMI_Lit:
        UpdateLitUnlitConstant(1);
        break;
    case EViewModeIndex::VMI_Wireframe:
    case EViewModeIndex::VMI_Unlit:
    case EViewModeIndex::VMI_SceneDepth:
    case EViewModeIndex::VMI_Velocity:
        UpdateLitUnlitConstant(0);
        break;
    default:
        break;
    }
}

void FRenderer::RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const
{
    uint32 Stride = sizeof(FStaticMeshVertex);
    uint32 Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &Offset);
    Graphics->DeviceContext->Draw(numVertices, 0);
}

void FRenderer::RenderPrimitive(ID3D11Buffer* pVertexBuffer, ID3D11Buffer* pIndexBuffer, UINT numIndices) const
{
    uint32 Stride = sizeof(FStaticMeshVertex);
    uint32 Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &Offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

void FRenderer::RenderPrimitive(OBJ::FStaticMeshRenderData* RenderData, TArray<FStaticMaterial*> Materials, TArray<UMaterial*> OverrideMaterial, int SelectedSubMeshIndex = -1) const
{
    uint32 Stride = sizeof(FStaticMeshVertex);
    uint32 Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &RenderData->VertexBuffer, &Stride, &Offset);

    if (RenderData->IndexBuffer)
    {
        Graphics->DeviceContext->IASetIndexBuffer(RenderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    }

    if (RenderData->MaterialSubsets.Num() == 0)
    {
        // no submesh
        Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
    }

    for (int subMeshIndex = 0; subMeshIndex < RenderData->MaterialSubsets.Num(); subMeshIndex++)
    {
        int materialIndex = RenderData->MaterialSubsets[subMeshIndex].MaterialIndex;

        subMeshIndex == SelectedSubMeshIndex ? UpdateSubMeshConstant(true) : UpdateSubMeshConstant(false);

        if (OverrideMaterial[materialIndex])
        {
            UpdateMaterial(OverrideMaterial[materialIndex]->GetMaterialInfo());
        }
        else
        {
            UpdateMaterial(Materials[materialIndex]->Material->GetMaterialInfo());
        }

        if (RenderData->IndexBuffer)
        {
            // index draw
            uint64 startIndex = RenderData->MaterialSubsets[subMeshIndex].IndexStart;
            uint64 indexCount = RenderData->MaterialSubsets[subMeshIndex].IndexCount;
            Graphics->DeviceContext->DrawIndexed(indexCount, startIndex, 0);
        }
    }
}

void FRenderer::RenderTexturedModelPrimitive(
    ID3D11Buffer* pVertexBuffer, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* InTextureSRV,
    ID3D11SamplerState* InSamplerState
) const
{
    if (!InTextureSRV || !InSamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    if (numIndices <= 0)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "numIndices Error");
    }

    uint32 Stride = sizeof(FStaticMeshVertex);
    uint32 Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &Offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    Graphics->DeviceContext->PSSetShaderResources(0, 1, &InTextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &InSamplerState);

    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

ID3D11Buffer* FRenderer::CreateVertexBuffer(FStaticMeshVertex* vertices, UINT byteWidth) const
{
    // 2. Create a vertex buffer
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD = {vertices};

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

ID3D11Buffer* FRenderer::CreateVertexBuffer(const TArray<FStaticMeshVertex>& vertices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD;
    vertexbufferSRD.pSysMem = vertices.GetData();

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

ID3D11Buffer* FRenderer::CreateIndexBuffer(uint32* Indices, UINT ByteWidth) const
{
    D3D11_BUFFER_DESC IndexBufferDesc = {};
    IndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IndexBufferDesc.ByteWidth = ByteWidth;

    D3D11_SUBRESOURCE_DATA IndexBufferData = {};
    IndexBufferData.pSysMem = Indices;

    ID3D11Buffer* indexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&IndexBufferDesc, &IndexBufferData, &indexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "IndexBuffer Creation failed");
    }
    return indexBuffer;
}

ID3D11Buffer* FRenderer::CreateIndexBuffer(const TArray<uint32>& Indices, UINT ByteWidth) const
{
    D3D11_BUFFER_DESC IndexBufferDesc = {};
    IndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IndexBufferDesc.ByteWidth = ByteWidth;

    D3D11_SUBRESOURCE_DATA IndexBufferData = {};
    IndexBufferData.pSysMem = Indices.GetData();

    ID3D11Buffer* indexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&IndexBufferDesc, &IndexBufferData, &indexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "IndexBuffer Creation failed");
    }
    return indexBuffer;
}

void FRenderer::CreateConstantBuffer()
{
    D3D11_BUFFER_DESC ConstantBufferDesc = {};
    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    ConstantBufferDesc.ByteWidth = sizeof(FObjectConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &ObjectConstantBuffer);

    ConstantBufferDesc.ByteWidth = sizeof(FViewConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &ViewConstantBuffer);

    ConstantBufferDesc.ByteWidth = sizeof(FProjectionConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &ProjectionConstantBuffer);

    ConstantBufferDesc.ByteWidth = sizeof(FGridParameters) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &GridConstantBuffer);

    ConstantBufferDesc.ByteWidth = sizeof(FGridParameters) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &GridConstantBuffer);

    ConstantBufferDesc.ByteWidth = sizeof(FPrimitiveCounts) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &LinePrimitiveBuffer);

    ConstantBufferDesc.ByteWidth = sizeof(FMaterialConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &MaterialConstantBuffer);

    ConstantBufferDesc.ByteWidth = sizeof(FSubMeshConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &SubMeshConstantBuffer);

    ConstantBufferDesc.ByteWidth = sizeof(FTextureConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &TextureConstantBuffer);

    ConstantBufferDesc.ByteWidth = sizeof(FExponentialHeightFogConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &ExponentialConstantBuffer);

    ConstantBufferDesc.ByteWidth = sizeof(FMotionBlurConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &MotionBlurConstantBuffer);
}

void FRenderer::CreateLightingBuffer()
{
    D3D11_BUFFER_DESC ConstantBufferDesc = {};
    ConstantBufferDesc.ByteWidth = sizeof(FLighting);
    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &LightingBuffer);
}

void FRenderer::CreateLitUnlitBuffer()
{
    D3D11_BUFFER_DESC ConstantBufferDesc = {};
    ConstantBufferDesc.ByteWidth = sizeof(FLitUnlitConstants);
    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &FlagBuffer);
}

void FRenderer::ReleaseConstantBuffer()
{
    ReleaseBuffer(ObjectConstantBuffer);
    ReleaseBuffer(ViewConstantBuffer);
    ReleaseBuffer(ProjectionConstantBuffer);
    ReleaseBuffer(LightingBuffer);
    ReleaseBuffer(FlagBuffer);
    ReleaseBuffer(MaterialConstantBuffer);
    ReleaseBuffer(SubMeshConstantBuffer);
    ReleaseBuffer(TextureConstantBuffer);
    ReleaseBuffer(ExponentialConstantBuffer);
    ReleaseBuffer(MotionBlurConstantBuffer);
}

void FRenderer::UpdateObjectBuffer(const FMatrix& PrevModelMatrix, const FMatrix& ModelMatrix, const FMatrix& InverseTransposedNormal, FVector4 UUIDColor, bool IsSelected) const
{
    if (ObjectConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;

        Graphics->DeviceContext->Map(ObjectConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
        if (FObjectConstants* constants = static_cast<FObjectConstants*>(ConstantBufferMSR.pData))
        {
            constants->ModelMatrix = ModelMatrix;
            constants->PrevModelMatrix = PrevModelMatrix;
            constants->ModelMatrixInverseTranspose = InverseTransposedNormal;
            constants->UUIDColor = UUIDColor;
            constants->IsSelected = IsSelected;
        }
        Graphics->DeviceContext->Unmap(ObjectConstantBuffer, 0);
    }
}

void FRenderer::UpdateViewBuffer(const FMatrix& PrevViewMatrix, const FMatrix& ViewMatrix, const FVector& ViewLocation) const
{
    if (ViewConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;

        Graphics->DeviceContext->Map(ViewConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
        if (FViewConstants* constants = static_cast<FViewConstants*>(ConstantBufferMSR.pData))
        {
            constants->ViewMatrix = ViewMatrix;
            constants->InvViewMatrix = FMatrix::Inverse(ViewMatrix);
            constants->PrevViewMatrix = PrevViewMatrix;
            constants->ViewLocation = ViewLocation;
        }
        Graphics->DeviceContext->Unmap(ViewConstantBuffer, 0);
    }
}

void FRenderer::UpdateProjectionBuffer(const FMatrix& PrevProjectionMatrix, const FMatrix& ProjectionMatrix, float NearClip, float FarClip) const
{
    if (ProjectionConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;

        Graphics->DeviceContext->Map(ProjectionConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
        if (FProjectionConstants* constants = static_cast<FProjectionConstants*>(ConstantBufferMSR.pData))
        {
            constants->ProjectionMatrix = ProjectionMatrix;
            constants->InvProjectionMatrix = FMatrix::Inverse(ProjectionMatrix);
            constants->PrevProjectionMatrix = PrevProjectionMatrix;
            constants->NearClip = NearClip;
            constants->FarClip = FarClip;
        }
        Graphics->DeviceContext->Unmap(ProjectionConstantBuffer, 0);
    }
}

void FRenderer::UpdateLightBuffer(ULevel* CurrentLevel) const
{
    if (!LightingBuffer || !CurrentLevel) return;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(LightingBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FLighting* constants = static_cast<FLighting*>(mappedResource.pData))
    {
        constants->FireBallCount = FireBalls.Num();

        for (int i = 0; i < constants->FireBallCount; i++)
        {
            UFireBallComponent* FireBall = FireBalls[i];
            constants->FireBalls[i].Position = FireBall->GetWorldLocation();
            constants->FireBalls[i].Radius = FireBall->GetRadius();
            constants->FireBalls[i].Intensity = FireBall->GetIntensity();
            constants->FireBalls[i].RadiusFallOff = FireBall->GetRadiusFallOff();
            constants->FireBalls[i].Color = FireBall->GetColor();
        }

        // 나머지 슬롯 초기화
        for (int i = constants->FireBallCount; i < constants->MAX_FIREBALLS; i++)
        {
            constants->FireBalls[i].Radius = 0.0f;
        }
    }
    Graphics->DeviceContext->Unmap(LightingBuffer, 0);
}

void FRenderer::UpdateMaterial(const FObjMaterialInfo& MaterialInfo) const
{
    if (MaterialConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;

        Graphics->DeviceContext->Map(MaterialConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        if (FMaterialConstants* constants = static_cast<FMaterialConstants*>(ConstantBufferMSR.pData))
        {
            constants->DiffuseColor = MaterialInfo.Diffuse;
            constants->TransparencyScalar = MaterialInfo.TransparencyScalar;
            constants->AmbientColor = MaterialInfo.Ambient;
            constants->DensityScalar = MaterialInfo.DensityScalar;
            constants->SpecularColor = MaterialInfo.Specular;
            constants->SpecularScalar = MaterialInfo.SpecularScalar;
            constants->EmmisiveColor = MaterialInfo.Emissive;
        }
        Graphics->DeviceContext->Unmap(MaterialConstantBuffer, 0);
    }

    if (MaterialInfo.bHasTexture == true)
    {
        std::shared_ptr<FTexture> texture = FEngineLoop::ResourceManager.GetTexture(MaterialInfo.DiffuseTexturePath);
        Graphics->DeviceContext->PSSetShaderResources(0, 1, &texture->TextureSRV);
        Graphics->DeviceContext->PSSetSamplers(0, 1, &texture->SamplerState);
    }
    else
    {
        ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
        ID3D11SamplerState* nullSampler[1] = {nullptr};

        Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);
        Graphics->DeviceContext->PSSetSamplers(0, 1, nullSampler);
    }
}

void FRenderer::UpdateLitUnlitConstant(int isLit) const
{
    if (FlagBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR;
        Graphics->DeviceContext->Map(FlagBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        if (FLitUnlitConstants* constants = static_cast<FLitUnlitConstants*>(constantbufferMSR.pData))
        {
            constants->isLit = isLit;
        }
        Graphics->DeviceContext->Unmap(FlagBuffer, 0);
    }
}

void FRenderer::UpdateSubMeshConstant(bool isSelected) const
{
    if (SubMeshConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR;
        Graphics->DeviceContext->Map(SubMeshConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        if (FSubMeshConstants* constants = static_cast<FSubMeshConstants*>(constantbufferMSR.pData))
        {
            constants->isSelectedSubMesh = isSelected;
        }
        Graphics->DeviceContext->Unmap(SubMeshConstantBuffer, 0);
    }
}

void FRenderer::UpdateTextureConstant(float UOffset, float VOffset)
{
    if (TextureConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE MappedSubresource;
        Graphics->DeviceContext->Map(TextureConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubresource);
        if (FTextureConstants* constants = static_cast<FTextureConstants*>(MappedSubresource.pData))
        {
            constants->UOffset = UOffset;
            constants->VOffset = VOffset;
        }
        Graphics->DeviceContext->Unmap(TextureConstantBuffer, 0);
    }
}

void FRenderer::UpdateExponentialHeightFogConstant(UExponentialHeightFogComponent* ExponentialHeightFogComp)
{
    if (ExponentialConstantBuffer) {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR; // GPU �� �޸� �ּ� ����
        Graphics->DeviceContext->Map(ExponentialConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        FExponentialHeightFogConstants* constants = (FExponentialHeightFogConstants*)constantbufferMSR.pData; //GPU �޸� ���� ����
        {
            constants->FogColor = ExponentialHeightFogComp->FogColor;
            constants->FogDensity = ExponentialHeightFogComp->FogDensity;
            constants->FogFalloff = ExponentialHeightFogComp->FogFalloff;
            constants->FogHeight = ExponentialHeightFogComp->FogHeight;
            constants->FogStartDistance = ExponentialHeightFogComp->FogStartDistance;
            constants->FogEndDistance = ExponentialHeightFogComp->FogEndDistance;
            constants->DistanceFogIntensity = ExponentialHeightFogComp->DistanceFogIntensity;
        }
        Graphics->DeviceContext->Unmap(ExponentialConstantBuffer, 0);
    }
}

void FRenderer::UpdateMotionBlurConstant(UMotionBlurComponent* MotionBlurComponent, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    if (MotionBlurComponent) {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR; // GPU �� �޸� �ּ� ����
        Graphics->DeviceContext->Map(MotionBlurConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        FMotionBlurConstants* constants = static_cast<FMotionBlurConstants*>(constantbufferMSR.pData);
        {
            constants->ScreenSizeX = ActiveViewport->Viewport->GetViewport().Width;
            constants->ScreenSizeY = ActiveViewport->Viewport->GetViewport().Height;
            constants->MaxBlurPixels = MotionBlurComponent->MaxBlurPixels;
            constants->VelocityScale = MotionBlurComponent->VelocityScale;
            constants->DepthThreshold = MotionBlurComponent->DepthThreshold;
        }
        Graphics->DeviceContext->Unmap(MotionBlurConstantBuffer, 0);
    }
}

void FRenderer::CreateFontShader()
{
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    HRESULT hr;
    hr = D3DCompileFromFile(L"Shaders/FontVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &VertexShaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "VertexShader Error");
    }
    Graphics->Device->CreateVertexShader(
        VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &FontVertexShader
    );

    hr = D3DCompileFromFile(L"Shaders/FontPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &PixelShaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "PixelShader Error");
    }
    Graphics->Device->CreatePixelShader(
        PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &FontPixelShader
    );

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &FontInputLayout
    );

    VertexShaderCSO->Release();
    PixelShaderCSO->Release();
}

void FRenderer::ReleaseFontShader()
{
    ReleaseShader(FontVertexShader);
    ReleaseShader(FontPixelShader);
    if (FontInputLayout)
    {
        FontInputLayout->Release();
        FontInputLayout = nullptr;
    }
}

void FRenderer::PrepareFontShader() const
{
    Graphics->DeviceContext->VSSetShader(FontVertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(FontPixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(FontInputLayout);
}

void FRenderer::CreateTextureShader()
{
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    HRESULT hr;
    hr = D3DCompileFromFile(L"Shaders/TextureVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &VertexShaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "VertexShader Error");
    }
    Graphics->Device->CreateVertexShader(
        VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &TextureVertexShader
    );

    hr = D3DCompileFromFile(L"Shaders/TexturePixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &PixelShaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "PixelShader Error");
    }
    Graphics->Device->CreatePixelShader(
        PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &TexturePixelShader
    );

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &TextureInputLayout
    );

    VertexShaderCSO->Release();
    PixelShaderCSO->Release();
}

void FRenderer::ReleaseTextureShader()
{
    if (TextureInputLayout)
    {
        TextureInputLayout->Release();
        TextureInputLayout = nullptr;
    }

    ReleaseShader(TexturePixelShader);

    ReleaseShader(TextureVertexShader);
    ReleaseBuffer(SubUVConstantBuffer);
    ReleaseBuffer(ObjectConstantBuffer);
}

void FRenderer::PrepareTextureShader() const
{
    Graphics->DeviceContext->VSSetShader(TextureVertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(TexturePixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(TextureInputLayout);
}

ID3D11Buffer* FRenderer::CreateVertexTextureBuffer(FVertexTexture* vertices, UINT byteWidth) const
{
    // 2. Create a vertex buffer
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_DYNAMIC; // will never be updated
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, nullptr, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

ID3D11Buffer* FRenderer::CreateIndexTextureBuffer(uint32* indices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC indexbufferdesc = {};
    indexbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
    indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexbufferdesc.ByteWidth = byteWidth;
    indexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer* indexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&indexbufferdesc, nullptr, &indexBuffer);
    if (FAILED(hr))
    {
        return nullptr;
    }
    return indexBuffer;
}

void FRenderer::RenderTexturePrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* _TextureSRV,
    ID3D11SamplerState* _SamplerState
) const
{
    if (!_TextureSRV || !_SamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    if (numIndices <= 0)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "numIndices Error");
    }

    uint32 Stride = sizeof(FVertexTexture);
    uint32 Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &Offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);

    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

//��Ʈ ��ġ������
void FRenderer::RenderTextPrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11ShaderResourceView* _TextureSRV, ID3D11SamplerState* _SamplerState
) const
{
    if (!_TextureSRV || !_SamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }

    uint32 Stride = sizeof(FVertexTexture);
    uint32 Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &Offset);

    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);

    ID3D11DepthStencilState* DepthStateDisable = Graphics->DepthStateDisable;
    Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);

    Graphics->DeviceContext->Draw(numVertices, 0);
}


ID3D11Buffer* FRenderer::CreateVertexBuffer(FVertexTexture* vertices, UINT byteWidth) const
{
    // 2. Create a vertex buffer
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD = {vertices};

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

void FRenderer::UpdateSubUVConstant(float _indexU, float _indexV) const
{
    if (SubUVConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

        Graphics->DeviceContext->Map(SubUVConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR); // update constant buffer every frame
        auto constants = static_cast<FSubUVConstant*>(constantbufferMSR.pData);
        {
            constants->indexU = _indexU;
            constants->indexV = _indexV;
        }
        Graphics->DeviceContext->Unmap(SubUVConstantBuffer, 0);
    }
}

void FRenderer::PrepareSubUVConstant() const
{
    if (SubUVConstantBuffer)
    {
        Graphics->DeviceContext->PSSetConstantBuffers(3, 1, &SubUVConstantBuffer);
    }
}

void FRenderer::SetDefaultConstantBuffer() const
{
    if (ObjectConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ObjectConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &ObjectConstantBuffer);
    }
    if (ViewConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &ViewConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &ViewConstantBuffer);
    }
    if (ProjectionConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(2, 1, &ProjectionConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(2, 1, &ProjectionConstantBuffer);
    }
}

void FRenderer::PreparePostProcess(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    Graphics->PreparePostProcess();

    ExponentialHeightFogComponent = nullptr;

    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Fog))
    {
        for (UExponentialHeightFogComponent* iter : TObjectRange<UExponentialHeightFogComponent>())
        {
            ExponentialHeightFogComponent = iter;
        }
    }

    MotionBlurComponent = nullptr;

    for (UMotionBlurComponent* iter : TObjectRange<UMotionBlurComponent>())
    {
        if (iter->bIsActive)
        {
            MotionBlurComponent = iter;
        }
    }
}

void FRenderer::PostProcess(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PreparePostProcess(ActiveViewport);

    // Draw Fog Quad
    if (ExponentialHeightFogComponent != nullptr) {
        // // TODO: Temp, 임시 코드
        Graphics->ClearAndSetRTV(Graphics->FogRTV, Graphics->ClearColor);
        UpdateExponentialHeightFogConstant(ExponentialHeightFogComponent);
        Graphics->DeviceContext->PSSetConstantBuffers(3, 1, &ExponentialConstantBuffer);

        Graphics->DeviceContext->PSSetShaderResources(0, 1, &Graphics->DepthStencilSRV);
        Graphics->DeviceContext->PSSetShaderResources(1, 1, &Graphics->WorldPosBufferSRV);

        Graphics->DeviceContext->PSSetShader(FogShader, nullptr, 0);
        Graphics->DeviceContext->PSSetSamplers(0, 1, &ScreenSamplerState);


        DrawFullScreenQuad();
    }


    // Draw Motion Blur
    if (MotionBlurComponent != nullptr) {
        // // TODO: Temp, 임시 코드
        Graphics->ClearAndSetRTV(Graphics->MotionBlurBufferRTV, Graphics->ZeroColor);
        UpdateMotionBlurConstant(MotionBlurComponent, ActiveViewport);
        Graphics->DeviceContext->PSSetConstantBuffers(3, 1, &MotionBlurConstantBuffer);

        Graphics->DeviceContext->PSSetShaderResources(0, 1, &Graphics->SceneBufferSRV);
        Graphics->DeviceContext->PSSetShaderResources(1, 1, &Graphics->VelocityBufferSRV);
        Graphics->DeviceContext->PSSetShaderResources(2, 1, &Graphics->NormalizedDepthSRV);
        Graphics->DeviceContext->PSSetShaderResources(3, 1, &Graphics->ViewNormalBufferSRV);

        Graphics->DeviceContext->PSSetShader(MotionBlurShader, nullptr, 0);
        Graphics->DeviceContext->PSSetSamplers(0, 1, &ScreenSamplerState);

        DrawFullScreenQuad();
    }
}

void FRenderer::PrepareLineShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexLineShader, nullptr, 0);

    Graphics->DeviceContext->PSSetShader(PixelLineShader, nullptr, 0);

    if (ObjectConstantBuffer && GridConstantBuffer)
    {
        Graphics->DeviceContext->VSSetShaderResources(2, 1, &pBBSRV);
        Graphics->DeviceContext->VSSetShaderResources(3, 1, &pConeSRV);
        Graphics->DeviceContext->VSSetShaderResources(4, 1, &pOBBSRV);

        Graphics->DeviceContext->VSSetConstantBuffers(3, 1, &GridConstantBuffer);
        Graphics->DeviceContext->VSSetConstantBuffers(4, 1, &LinePrimitiveBuffer);

        Graphics->DeviceContext->PSSetConstantBuffers(3, 1, &GridConstantBuffer);
    }
}

void FRenderer::CreateLineShader()
{
    ID3DBlob* VertexShaderLine;
    ID3DBlob* PixelShaderLine;

    HRESULT hr;
    hr = D3DCompileFromFile(L"Shaders/ShaderLine.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainVS", "vs_5_0", 0, 0, &VertexShaderLine, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "VertexShader Error");
    }
    Graphics->Device->CreateVertexShader(VertexShaderLine->GetBufferPointer(), VertexShaderLine->GetBufferSize(), nullptr, &VertexLineShader);

    hr = D3DCompileFromFile(L"Shaders/ShaderLine.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainPS", "ps_5_0", 0, 0, &PixelShaderLine, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "PixelShader Error");
    }
    Graphics->Device->CreatePixelShader(PixelShaderLine->GetBufferPointer(), PixelShaderLine->GetBufferSize(), nullptr, &PixelLineShader);


    VertexShaderLine->Release();
    PixelShaderLine->Release();
}

void FRenderer::ReleaseLineShader()
{
    ReleaseBuffer(GridConstantBuffer);
    ReleaseBuffer(LinePrimitiveBuffer);
    ReleaseShader(VertexLineShader);
    ReleaseShader(PixelLineShader);
}

ID3D11Buffer* FRenderer::CreateStaticVerticesBuffer() const
{
    FSimpleVertex vertices[2]{{0}, {0}};

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA vbInitData = {};
    vbInitData.pSysMem = vertices;
    ID3D11Buffer* pVertexBuffer = nullptr;
    HRESULT hr = Graphics->Device->CreateBuffer(&vbDesc, &vbInitData, &pVertexBuffer);
    return pVertexBuffer;
}

ID3D11Buffer* FRenderer::CreateBoundingBoxBuffer(UINT numBoundingBoxes) const
{
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(FBoundingBox) * numBoundingBoxes;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FBoundingBox);

    ID3D11Buffer* BoundingBoxBuffer = nullptr;
    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &BoundingBoxBuffer);
    return BoundingBoxBuffer;
}

ID3D11Buffer* FRenderer::CreateOBBBuffer(UINT numBoundingBoxes) const
{
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(FOrientedBoundingBox) * numBoundingBoxes;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FOrientedBoundingBox);

    ID3D11Buffer* BoundingBoxBuffer = nullptr;
    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &BoundingBoxBuffer);
    return BoundingBoxBuffer;
}

ID3D11Buffer* FRenderer::CreateConeBuffer(UINT numCones) const
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(FCone) * numCones;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FCone);

    ID3D11Buffer* ConeBuffer = nullptr;
    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &ConeBuffer);
    return ConeBuffer;
}

ID3D11ShaderResourceView* FRenderer::CreateBoundingBoxSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numBoundingBoxes;


    Graphics->Device->CreateShaderResourceView(pBoundingBoxBuffer, &srvDesc, &pBBSRV);
    return pBBSRV;
}

ID3D11ShaderResourceView* FRenderer::CreateOBBSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numBoundingBoxes;
    Graphics->Device->CreateShaderResourceView(pBoundingBoxBuffer, &srvDesc, &pOBBSRV);
    return pOBBSRV;
}

ID3D11ShaderResourceView* FRenderer::CreateConeSRV(ID3D11Buffer* pConeBuffer, UINT numCones)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numCones;


    Graphics->Device->CreateShaderResourceView(pConeBuffer, &srvDesc, &pConeSRV);
    return pConeSRV;
}

void FRenderer::UpdateBoundingBoxBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FBoundingBox>& BoundingBoxes, int numBoundingBoxes) const
{
    if (!pBoundingBoxBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pBoundingBoxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FBoundingBox*>(mappedResource.pData);
    for (int i = 0; i < BoundingBoxes.Num(); ++i)
    {
        pData[i] = BoundingBoxes[i];
    }
    Graphics->DeviceContext->Unmap(pBoundingBoxBuffer, 0);
}

void FRenderer::UpdateOBBBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FOrientedBoundingBox>& BoundingBoxes, int numBoundingBoxes) const
{
    if (!pBoundingBoxBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pBoundingBoxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FOrientedBoundingBox*>(mappedResource.pData);
    for (int i = 0; i < BoundingBoxes.Num(); ++i)
    {
        pData[i] = BoundingBoxes[i];
    }
    Graphics->DeviceContext->Unmap(pBoundingBoxBuffer, 0);
}

void FRenderer::UpdateConesBuffer(ID3D11Buffer* pConeBuffer, const TArray<FCone>& Cones, int numCones) const
{
    if (!pConeBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pConeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FCone*>(mappedResource.pData);
    for (int i = 0; i < Cones.Num(); ++i)
    {
        pData[i] = Cones[i];
    }
    Graphics->DeviceContext->Unmap(pConeBuffer, 0);
}

void FRenderer::OnEndRender()
{
    for (const auto& Obj : StaticMeshObjs)
    {
        Obj->UpdatePrevTransform();
    }
    ClearRenderArr();
}

void FRenderer::UpdateGridConstantBuffer(const FGridParameters& gridParams) const
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(GridConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        memcpy(mappedResource.pData, &gridParams, sizeof(FGridParameters));
        Graphics->DeviceContext->Unmap(GridConstantBuffer, 0);
    }
    else
    {
        UE_LOG(LogLevel::Warning, "gridParams ���� ����");
    }
}

void FRenderer::UpdateLinePrimitveCountBuffer(int numBoundingBoxes, int numCones) const
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(LinePrimitiveBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = static_cast<FPrimitiveCounts*>(mappedResource.pData);
    pData->BoundingBoxCount = numBoundingBoxes;
    pData->ConeCount = numCones;
    Graphics->DeviceContext->Unmap(LinePrimitiveBuffer, 0);
}

void FRenderer::RenderBatch(
    const FGridParameters& gridParam, ID3D11Buffer* pVertexBuffer, int boundingBoxCount, int coneCount, int coneSegmentCount, int obbCount
) const
{
    UINT stride = sizeof(FSimpleVertex);
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    UINT vertexCountPerInstance = 2;
    UINT instanceCount = gridParam.numGridLines + 3 + (boundingBoxCount * 12) + (coneCount * (2 * coneSegmentCount)) + (12 * obbCount);
    Graphics->DeviceContext->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void FRenderer::PrepareRender(ULevel* Level)
{
    // TODO: 임시로 진행한 코드이므로, 수정 필수

    TArray<USceneComponent*> Ss;
    for (const auto& A : Level->GetActors())
    {
        Ss.Add(A->GetRootComponent());
        TArray<USceneComponent*> temp;
        A->GetRootComponent()->GetChildrenComponents(temp);
        Ss + temp;
    }


    for (const USceneComponent* iter : TObjectRange<USceneComponent>())
    {
        if (UGizmoBaseComponent* pGizmoComp = Cast<UGizmoBaseComponent>(iter))
        {
            GizmoObjs.Add(pGizmoComp);
        }
        if (UTextRenderComponent* TextRenderComp = Cast<UTextRenderComponent>(iter))
        {
            TextObjs.Add(TextRenderComp);
        }
        if (ULightComponentBase* pLightComp = Cast<ULightComponentBase>(iter))
        {
            LightObjs.Add(pLightComp);
        }
        if (UFireBallComponent* pFireBallComp = Cast<UFireBallComponent>(iter))
        {
            FireBalls.Add(pFireBallComp);
        }
    }

    for (const auto iter : Ss)
    {
        if (UStaticMeshComponent* pStaticMeshComp = Cast<UStaticMeshComponent>(iter))
        {
            if (!Cast<UGizmoBaseComponent>(iter))
                StaticMeshObjs.Add(pStaticMeshComp);
        }

        if (UBillboardComponent* pBillboardComp = Cast<UBillboardComponent>(iter))
        {
            if (UTextBillboardComponent* TextBillboardComp = Cast<UTextBillboardComponent>(iter))
            {
                TextObjs.Add(TextBillboardComp);
            }
            else
            {
                BillboardObjs.Add(pBillboardComp);
            }
        }

    }

    SetDefaultConstantBuffer();
}

void FRenderer::RenderScene(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    Graphics->DeviceContext->RSSetViewports(1, &ActiveViewport->GetD3DViewport());
    Graphics->ChangeRasterizer(ActiveViewport->GetViewMode());
    ChangeViewMode(ActiveViewport->GetViewMode());

    // TODO: 임시로 여기에서 View 상수 버퍼와 Projection 상수 버퍼 업데이트
    UpdateViewBuffer(ActiveViewport->GetPrevViewMatrix(), ActiveViewport->GetViewMatrix(), ActiveViewport->ViewTransformPerspective.GetLocation());
    UpdateProjectionBuffer(ActiveViewport->GetPrevProjectionMatrix(), ActiveViewport->GetProjectionMatrix(), ActiveViewport->nearPlane, ActiveViewport->farPlane);

    if(FireBalls.Num() > 0)
    {
        UpdateLightBuffer(Level);
    }

    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
    {
        if (StaticMeshObjs.Num() > 0)
        {
            RenderStaticMeshes(Level, ActiveViewport);
        }
    }

    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Gizmo))
    {
        if (GizmoObjs.Num() > 0)
        {
            RenderGizmos(Level, ActiveViewport);
        }

        UPrimitiveBatch::GetInstance().RenderBatch(ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix());
    }

    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))
    {
        if (BillboardObjs.Num() > 0)
        {
            RenderBillboards(Level, ActiveViewport);
        }
        if (TextObjs.Num() > 0)
        {
            RenderTexts(Level, ActiveViewport);
        }
    }

    if (LightObjs.Num() > 0)
    {
        RenderLight(Level, ActiveViewport);
    }
}

void FRenderer::SampleAndProcessSRV(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    // Depth - Normalize
    Graphics->PrepareDepthMap();

    Graphics->DeviceContext->PSSetShaderResources(0, 1, &Graphics->DepthStencilSRV);

    Graphics->DeviceContext->PSSetShader(DepthShader, nullptr, 0);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &ScreenSamplerState);

    DrawFullScreenQuad();
}

void FRenderer::RenderFullScreenQuad(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    Graphics->PrepareFinal();

    if (ViewModeFlags == EViewModeIndex::VMI_SceneDepth)
    {
        // TODO: 텍스처 슬롯 개수만큼 null -> 임시코드임
        int n = 2;
        for (int i = 0; i < n; i++)
        {
            ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
            Graphics->DeviceContext->PSSetShaderResources(i, 1, nullSRV);
        }

        // TODO: Temp, 임시 코드
        Graphics->DeviceContext->PSSetShaderResources(0, 1, &Graphics->NormalizedDepthSRV);
    }
    else if (ViewModeFlags == EViewModeIndex::VMI_Velocity)
    {
        // TODO: 텍스처 슬롯 개수만큼 null -> 임시코드임
        int n = 2;
        for (int i = 0; i < n; i++)
        {
            ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
            Graphics->DeviceContext->PSSetShaderResources(i, 1, nullSRV);
        }

        // TODO: Temp, 임시 코드
        Graphics->DeviceContext->PSSetShaderResources(0, 1, &Graphics->VelocityBufferSRV);
    }
    else
    {
        Graphics->DeviceContext->PSSetShaderResources(0, 1, &Graphics->SceneBufferSRV);

        ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
        // TODO: Temp 코드
        if (ExponentialHeightFogComponent)
        {
            Graphics->DeviceContext->PSSetShaderResources(2, 1, &Graphics->FogSRV);
        }
        else
        {
            Graphics->DeviceContext->PSSetShaderResources(2, 1, nullSRV);
        }

        // TODO: Temp 코드
        if (MotionBlurComponent)
        {
            Graphics->DeviceContext->PSSetShaderResources(1, 1, &Graphics->MotionBlurBufferSRV);
        }
        else
        {
            Graphics->DeviceContext->PSSetShaderResources(1, 1, nullSRV);
        }
    }
    Graphics->DeviceContext->PSSetShader(FinalPixelShader, nullptr, 0);

    Graphics->DeviceContext->PSSetSamplers(0, 1, &ScreenSamplerState);

    DrawFullScreenQuad();
}

void FRenderer::DrawFullScreenQuad()
{
    Graphics->DeviceContext->VSSetShader(FinalVertexShader, nullptr, 0);

    // (float3(), 0, 0), (float3(), 1, 0), (float3(), 0, 1), (float3(), 1, 0), (float3(), 1, 1), (float3(), 0, 1))
    Graphics->DeviceContext->IASetInputLayout(nullptr);  // 입력 레이아웃 필요 없음
    Graphics->DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    Graphics->DeviceContext->Draw(6, 0);
}

void FRenderer::ClearRenderArr()
{
    StaticMeshObjs.Empty();
    GizmoObjs.Empty();
    TextObjs.Empty();
    LightObjs.Empty();
    FireBalls.Empty();
    BillboardObjs.Empty();
}

void FRenderer::RenderStaticMeshes(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareShader();
    for (UStaticMeshComponent* StaticMeshComp : StaticMeshObjs)
    {
        FMatrix PrevModel = JungleMath::CreateModelMatrix(
            StaticMeshComp->GetPrevWorldLocation(),
            StaticMeshComp->GetPrevWorldRotation(),
            StaticMeshComp->GetPrevWorldScale()
        );

        FMatrix Model = JungleMath::CreateModelMatrix(
            StaticMeshComp->GetWorldLocation(),
            StaticMeshComp->GetWorldRotation(),
            StaticMeshComp->GetWorldScale()
        );
        // 노말 회전시 필요 행렬
        FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        FVector4 UUIDColor = StaticMeshComp->EncodeUUID() / 255.0f;

        UpdateObjectBuffer(PrevModel, Model, NormalMatrix, UUIDColor, Level->GetSelectedActor() == StaticMeshComp->GetOwner());

        UpdateTextureConstant(0, 0);

        if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        {
            UPrimitiveBatch::GetInstance().RenderAABB(
                StaticMeshComp->GetBoundingBox(),
                StaticMeshComp->GetWorldLocation(),
                Model
            );
        }

        if (!StaticMeshComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = StaticMeshComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        RenderPrimitive(renderData, StaticMeshComp->GetStaticMesh()->GetMaterials(), StaticMeshComp->GetOverrideMaterials(), StaticMeshComp->GetselectedSubMeshIndex());
    }
}

void FRenderer::RenderGizmos(const ULevel* Level, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    if (!Level->GetSelectedActor())
    {
        return;
    }

    PrepareShader();

#pragma region GizmoDepth
    ID3D11DepthStencilState* DepthStateDisable = Graphics->DepthStateDisable;
    Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
#pragma endregion GizmoDepth

    //  fill solid,  Wirframe 에서도 제대로 렌더링되기 위함
    Graphics->DeviceContext->RSSetState(FEngineLoop::GraphicDevice.RasterizerStateSOLID);

    for (auto GizmoComp : GizmoObjs)
    {
        if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowX ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowY ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowZ)
            && Level->GetEditorPlayer()->GetControlMode() != CM_TRANSLATION)
            continue;
        else if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleX ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleY ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleZ)
            && Level->GetEditorPlayer()->GetControlMode() != CM_SCALE)
            continue;
        else if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleX ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleY ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleZ)
            && Level->GetEditorPlayer()->GetControlMode() != CM_ROTATION)
            continue;
        FMatrix Model = JungleMath::CreateModelMatrix(GizmoComp->GetWorldLocation(),
            GizmoComp->GetWorldRotation(),
            GizmoComp->GetWorldScale()
        );
        FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        FVector4 UUIDColor = GizmoComp->EncodeUUID() / 255.0f;

        UpdateObjectBuffer(Model, Model, NormalMatrix, UUIDColor, GizmoComp == Level->GetPickingGizmo());

        if (!GizmoComp->GetStaticMesh())
        {
            continue;
        }

        OBJ::FStaticMeshRenderData* renderData = GizmoComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr)
        {
            continue;
        }

        RenderPrimitive(renderData, GizmoComp->GetStaticMesh()->GetMaterials(), GizmoComp->GetOverrideMaterials());
    }

    Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());

#pragma region GizmoDepth
    ID3D11DepthStencilState* originalDepthState = Graphics->DepthStencilState;
    Graphics->DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
#pragma endregion GizmoDepth
}

void FRenderer::RenderBillboards(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareTextureShader();
    PrepareSubUVConstant();

    for (auto BillboardComp : BillboardObjs)
    {
        UpdateSubUVConstant(BillboardComp->finalIndexU, BillboardComp->finalIndexV);

        FMatrix Model = BillboardComp->CreateBillboardMatrix();
        FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        FVector4 UUIDColor = BillboardComp->EncodeUUID() / 255.0f;

        UpdateObjectBuffer(Model, Model, NormalMatrix, UUIDColor, BillboardComp == Level->GetPickingGizmo());

        if (UParticleSubUVComp* SubUVParticle = Cast<UParticleSubUVComp>(BillboardComp))
        {
            RenderTexturePrimitive(
                SubUVParticle->vertexSubUVBuffer, SubUVParticle->numTextVertices,
                SubUVParticle->indexTextureBuffer, SubUVParticle->numIndices, SubUVParticle->Texture->TextureSRV, SubUVParticle->Texture->SamplerState
            );
        }
        else
        {
            RenderTexturePrimitive(
                BillboardComp->vertexTextureBuffer, BillboardComp->numVertices,
                BillboardComp->indexTextureBuffer, BillboardComp->numIndices, BillboardComp->Texture->TextureSRV, BillboardComp->Texture->SamplerState
            );
        }
    }
    PrepareShader();
}

void FRenderer::RenderTexts(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareFontShader();
    PrepareSubUVConstant();

    for (auto TextComps : TextObjs)
    {
        if (UTextBillboardComponent* TextBillboard = Cast<UTextBillboardComponent>(TextComps))
        {
            UpdateSubUVConstant(TextBillboard->finalIndexU, TextBillboard->finalIndexV);

            FMatrix Model = TextBillboard->CreateBillboardMatrix();
            FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
            FVector4 UUIDColor = TextComps->EncodeUUID() / 255.0f;

            UpdateObjectBuffer(Model, Model, NormalMatrix, UUIDColor, TextComps == Level->GetPickingGizmo());

            FEngineLoop::Renderer.RenderTextPrimitive(
                TextBillboard->vertexTextBuffer, TextBillboard->numTextVertices,
                TextBillboard->Texture->TextureSRV, TextBillboard->Texture->SamplerState
            );
        }
        else if (UTextRenderComponent* TextRender = Cast<UTextRenderComponent>(TextComps))
        {
            UpdateSubUVConstant(TextRender->finalIndexU, TextRender->finalIndexV);

            FMatrix Model = JungleMath::CreateModelMatrix(
                TextRender->GetWorldLocation(),
                TextRender->GetWorldRotation(),
                TextRender->GetWorldScale()
            );
            FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
            FVector4 UUIDColor = TextComps->EncodeUUID() / 255.0f;
            UpdateObjectBuffer(Model, Model, NormalMatrix, UUIDColor, TextComps == Level->GetPickingGizmo());

            FEngineLoop::Renderer.RenderTextPrimitive(
                TextRender->vertexTextBuffer, TextRender->numTextVertices,
                TextRender->Texture->TextureSRV, TextRender->Texture->SamplerState
            );
        }
    }
    PrepareShader();
}

void FRenderer::RenderLight(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    for (auto Light : LightObjs)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(Light->GetWorldLocation(), Light->GetWorldRotation(), {1, 1, 1});
        UPrimitiveBatch::GetInstance().AddCone(Light->GetWorldLocation(), Light->GetRadius(), 15, 140, Light->GetColor(), Model);
        UPrimitiveBatch::GetInstance().RenderOBB(Light->GetBoundingBox(), Light->GetWorldLocation(), Model);
    }
}
