#include "Engine/Source/Runtime/Engine/Level.h"

#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Engine/FLoaderOBJ.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Components/SkySphereComponent.h"
#include "Components/UFireBallComponent.h"



void ULevel::Initialize(EWorldType worldType)
{
    // TODO: Load Scene
    CreateBaseObject(worldType);
    //SpawnObject(OBJ_CUBE);

    FManagerOBJ::CreateStaticMesh("Assets/Street/Street.obj");
    FManagerOBJ::CreateStaticMesh("Assets/Woojae/Woojea1.obj");
    FManagerOBJ::CreateStaticMesh("Assets/Woojae/Woojea2.obj");
}

void ULevel::CreateBaseObject(EWorldType worldType)
{
    if (EditorPlayer == nullptr && worldType == EWorldType::Editor)
        EditorPlayer = FObjectFactory::ConstructObject<AEditorPlayer>();
    if (LocalGizmo == nullptr && worldType == EWorldType::Editor)
        LocalGizmo = FObjectFactory::ConstructObject<UTransformGizmo>();

    if (LocalGizmo != nullptr && worldType == EWorldType::PIE)
        LocalGizmo = nullptr;
    if (EditorPlayer != nullptr && worldType == EWorldType::PIE)
        EditorPlayer = nullptr;
}

void ULevel::ReleaseBaseObject()
{
    if (LocalGizmo)
    {
        delete LocalGizmo;
        LocalGizmo = nullptr;
    }

    if (EditorPlayer)
    {
        delete EditorPlayer;
        EditorPlayer = nullptr;
    }
}

void ULevel::Tick(float DeltaTime)
{
	if (EditorPlayer)
	{
	    EditorPlayer->Tick(DeltaTime);
	}
	if (LocalGizmo)
	{
	    LocalGizmo->Tick(DeltaTime);
	}

    // SpawnActor()에 의해 Actor가 생성된 경우, 여기서 BeginPlay 호출
    for (AActor* Actor : PendingBeginPlayActors)
    {
        Actor->BeginPlay();
    }
    PendingBeginPlayActors.Empty();

    // 매 틱마다 Actor->Tick(...) 호출
    for (AActor* Actor : ActorsArray)
    {
        Actor->Tick(DeltaTime);
    }
}

void ULevel::Release()
{
	for (AActor* Actor : ActorsArray)
	{
		Actor->EndPlay(EEndPlayReason::WorldTransition);
        TArray<UActorComponent*> Components = Actor->GetComponents();
        for (UActorComponent* Component : Components)
        {
            GUObjectArray.MarkRemoveObject(Component);
        }
        GUObjectArray.MarkRemoveObject(Actor);
    }
    ActorsArray.Empty();

    pickingGizmo = nullptr;
    ReleaseBaseObject();

    GUObjectArray.ProcessPendingDestroyObjects();
}

UObject* ULevel::Duplicate()
{
    // 새 객체 생성 및 얕은 복사
    UObject* NewObject = FObjectFactory::ConstructObject<ULevel>(this);

    // 서브 오브젝트는 깊은 복사로 별도 처리
    Cast<ULevel>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void ULevel::DuplicateSubObjects()
{
    Super::DuplicateSubObjects();
    TArray<AActor*> DuplicatedActors;

    for (auto& Actor : ActorsArray)
    {
        DuplicatedActors.Add(Cast<AActor>(Actor->Duplicate())); //TODO: 클래스 구별
    }
    PendingBeginPlayActors.Empty();

    SelectedActor = nullptr;
    pickingGizmo = nullptr;
    EditorPlayer = nullptr;
    LocalGizmo = nullptr;

    ActorsArray = DuplicatedActors;
}

bool ULevel::DestroyActor(AActor* ThisActor)
{
    if (ThisActor->GetLevel() == nullptr)
    {
        return false;
    }

    if (ThisActor->IsActorBeingDestroyed())
    {
        return true;
    }

    // 액터의 Destroyed 호출
    ThisActor->Destroyed();

    if (ThisActor->GetOwner())
    {
        ThisActor->SetOwner(nullptr);
    }

    // World에서 제거
    ActorsArray.Remove(ThisActor);

    // 제거 대기열에 추가
    GUObjectArray.MarkRemoveObject(ThisActor);
    return true;
}



void ULevel::AddActor(AActor* NewActor)
{
    ActorsArray.Add(NewActor);
    PendingBeginPlayActors.Add(NewActor);
}

void ULevel::SetPickingGizmo(UObject* Object)
{
    pickingGizmo = Cast<USceneComponent>(Object);
}