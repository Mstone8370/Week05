#include "TransformGizmo.h"
#include "GizmoArrowComponent.h"
#include "Define.h"
#include "GizmoCircleComponent.h"
#include "Actors/Player.h"
#include "GizmoRectangleComponent.h"
#include "Level.h"
#include "Engine/FLoaderOBJ.h"

UTransformGizmo::UTransformGizmo()
{
    FManagerOBJ::CreateStaticMesh("Assets/GizmoTranslationX.obj");
    FManagerOBJ::CreateStaticMesh("Assets/GizmoTranslationY.obj");
    FManagerOBJ::CreateStaticMesh("Assets/GizmoTranslationZ.obj");
    FManagerOBJ::CreateStaticMesh("Assets/GizmoRotationX.obj");
    FManagerOBJ::CreateStaticMesh("Assets/GizmoRotationY.obj");
    FManagerOBJ::CreateStaticMesh("Assets/GizmoRotationZ.obj");
    FManagerOBJ::CreateStaticMesh("Assets/GizmoScaleX.obj");
    FManagerOBJ::CreateStaticMesh("Assets/GizmoScaleY.obj");
    FManagerOBJ::CreateStaticMesh("Assets/GizmoScaleZ.obj");

    SetRootComponent(
        AddComponent<USceneComponent>()
    );

    UGizmoArrowComponent* locationX = AddComponent<UGizmoArrowComponent>();
    locationX->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"GizmoTranslationX.obj"));
	locationX->SetupAttachment(RootComponent);
    locationX->SetGizmoType(UGizmoBaseComponent::ArrowX);
	ArrowArr.Add(locationX);

    UGizmoArrowComponent* locationY = AddComponent<UGizmoArrowComponent>();
    locationY->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"GizmoTranslationY.obj"));
    locationY->SetupAttachment(RootComponent);
    locationY->SetGizmoType(UGizmoBaseComponent::ArrowY);
    ArrowArr.Add(locationY);

    UGizmoArrowComponent* locationZ = AddComponent<UGizmoArrowComponent>();
    locationZ->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"GizmoTranslationZ.obj"));
    locationZ->SetupAttachment(RootComponent);
    locationZ->SetGizmoType(UGizmoBaseComponent::ArrowZ);
    ArrowArr.Add(locationZ);

    UGizmoRectangleComponent* ScaleX = AddComponent<UGizmoRectangleComponent>();
    ScaleX->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"GizmoScaleX.obj"));
    ScaleX->SetupAttachment(RootComponent);
    ScaleX->SetGizmoType(UGizmoBaseComponent::ScaleX);
    RectangleArr.Add(ScaleX);

    UGizmoRectangleComponent* ScaleY = AddComponent<UGizmoRectangleComponent>();
    ScaleY->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"GizmoScaleY.obj"));
    ScaleY->SetupAttachment(RootComponent);
    ScaleY->SetGizmoType(UGizmoBaseComponent::ScaleY);
    RectangleArr.Add(ScaleY);

    UGizmoRectangleComponent* ScaleZ = AddComponent<UGizmoRectangleComponent>();
    ScaleZ->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"GizmoScaleZ.obj"));
    ScaleZ->SetupAttachment(RootComponent);
    ScaleZ->SetGizmoType(UGizmoBaseComponent::ScaleZ);
    RectangleArr.Add(ScaleZ);

    UGizmoCircleComponent* CircleX = AddComponent<UGizmoCircleComponent>();
    CircleX->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"GizmoRotationX.obj"));
    CircleX->SetupAttachment(RootComponent);
    CircleX->SetGizmoType(UGizmoBaseComponent::CircleX);
    CircleArr.Add(CircleX);

    UGizmoCircleComponent* CircleY = AddComponent<UGizmoCircleComponent>();
    CircleY->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"GizmoRotationY.obj"));
    CircleY->SetupAttachment(RootComponent);
    CircleY->SetGizmoType(UGizmoBaseComponent::CircleY);
    CircleArr.Add(CircleY);

    UGizmoCircleComponent* CircleZ = AddComponent<UGizmoCircleComponent>();
    CircleZ->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"GizmoRotationZ.obj"));
    CircleZ->SetupAttachment(RootComponent);
    CircleZ->SetGizmoType(UGizmoBaseComponent::CircleZ);
    CircleArr.Add(CircleZ);
}

void UTransformGizmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (USceneComponent* SceneComponent = GetLevel()->GetSelectedTempComponent())
    {
        SetActorLocation(SceneComponent->GetWorldLocation());
        if (GetLevel()->GetEditorPlayer()->GetCoordiMode() == CoordiMode::CDM_LOCAL)
        {
            // TODO: 임시로 RootComponent의 정보로 사용
            SetActorRotation(SceneComponent->GetWorldRotation());
        }
        else if (GetLevel()->GetEditorPlayer()->GetCoordiMode() == CoordiMode::CDM_WORLD)
            SetActorRotation(FVector(0.0f, 0.0f, 0.0f));
    }
}