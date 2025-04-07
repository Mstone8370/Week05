#include "GizmoArrowComponent.h"

#include "Level.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "Actors/Player.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"


UGizmoArrowComponent::UGizmoArrowComponent()
{
}

UGizmoArrowComponent::~UGizmoArrowComponent()
{
}

void UGizmoArrowComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UGizmoArrowComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}

// void UGizmoArrowComponent::Render()
// {
// #pragma region GizmoDepth
//     ID3D11DepthStencilState* DepthStateDisable = FEngineLoop::GraphicDevice.DepthStateDisable;
//     FEngineLoop::GraphicDevice.DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
// #pragma endregion GizmoDepth

//     if (!GetWorld()->GetPickingObj() || GetWorld()->GetEditorPlayer()->GetControlMode() != CM_TRANSLATION)
//         return;
//     if (!staticMesh) return;

//     OBJ::FStaticMeshRenderData* renderData = staticMesh->GetRenderData();
//     FMatrix Model = JungleMath::CreateModelMatrix(GetWorldLocation(), GetWorldRotation(), GetWorldScale());
//     FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
//     FVector4 UUIDColor = EncodeUUID() / 255.0f;
    
//     // 최종 MVP 행렬
//     FMatrix MVP = Model * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix() * GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetProjectionMatrix();
    
//     if (this == GetWorld()->GetPickingGizmo()) {
//         FEngineLoop::Renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
//     }
//     else
//         FEngineLoop::Renderer.UpdateConstant(MVP, NormalMatrix, UUIDColor, false);
    
//     FEngineLoop::GraphicDevice.DeviceContext->RSSetState(FEngineLoop::GraphicDevice.RasterizerStateSOLID); // fill solid로 렌더링.
//     // FEngineLoop::Renderer.RenderPrimitive(renderData, OverrideMaterials);
//     FEngineLoop::GraphicDevice.DeviceContext->RSSetState(FEngineLoop::GraphicDevice.GetCurrentRasterizer()); // 이전 레스터라이저 재설정.

// #pragma region GizmoDepth
//     ID3D11DepthStencilState* originalDepthState = FEngineLoop::GraphicDevice.DepthStencilState;
//     FEngineLoop::GraphicDevice.DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
// #pragma endregion GizmoDepth
// }
