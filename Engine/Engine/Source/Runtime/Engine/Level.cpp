#include "Engine/Source/Runtime/Engine/Level.h"

#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Engine/FLoaderOBJ.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Components/MotionBlurComponent.h"
#include "Components/SkySphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

void ULevel::Initialize(EWorldType worldType)
{
    // TODO: Load Scene
    CreateBaseObject(worldType);
    //SpawnObject(OBJ_CUBE);

    FManagerOBJ::CreateStaticMesh("Assets/Sphere.obj");
    FManagerOBJ::CreateStaticMesh("Assets/Cube.obj");
    FManagerOBJ::CreateStaticMesh("Assets/Fireball.obj");

    FManagerOBJ::CreateStaticMesh("Assets/Nessie/Nessie.obj");

    AActor* Nessie = SpawnActor<AActor>();
    UStaticMeshComponent* Mesh = Nessie->AddComponent<UStaticMeshComponent>();
    Mesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Nessie.obj"));
    Mesh->SetScale(FVector(5.f, 5.f, 5.f));
    Nessie->SetRootComponent(Mesh);

#ifndef _DEBUG
    FManagerOBJ::CreateStaticMesh("Assets/Street/Street.obj");
    FManagerOBJ::CreateStaticMesh("Assets/Woojae/Woojae1.obj");
    FManagerOBJ::CreateStaticMesh("Assets/Woojae/Woojae2.obj");

    AActor* Street1 = SpawnActor<AActor>();
    UStaticMeshComponent* Street1Mesh = Street1->AddComponent<UStaticMeshComponent>();
    Street1Mesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Street.obj"));
    Street1Mesh->SetLocation(FVector(6.05, -8.56, 0));
    Street1Mesh->SetRotation(FVector(0, 0, 90));
    Street1->SetRootComponent(Street1Mesh);

    AActor* Street2 = SpawnActor<AActor>();
    UStaticMeshComponent* Street2Mesh = Street2->AddComponent<UStaticMeshComponent>();
    Street2Mesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Street.obj"));
    Street2Mesh->SetLocation(FVector(0, 0, 0.02f));
    Street2Mesh->SetRotation(FVector(0, 0, 0));
    Street2->SetRootComponent(Street2Mesh);

    AActor* WJ1 = SpawnActor<AActor>();
    UStaticMeshComponent* WJ1Mesh = WJ1->AddComponent<UStaticMeshComponent>();
    WJ1Mesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Woojae1.obj"));
    WJ1Mesh->SetLocation(FVector(-1.4, 2.24, 0));
    WJ1Mesh->SetRotation(FVector(0, 0, -52.14));
    WJ1->SetRootComponent(WJ1Mesh);

    AActor* WJ2 = SpawnActor<AActor>();
    UStaticMeshComponent* WJ2Mesh = WJ2->AddComponent<UStaticMeshComponent>();
    WJ2Mesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Woojae2.obj"));
    WJ2Mesh->SetLocation(FVector(1.49, -9.82, 0));
    WJ2Mesh->SetRotation(FVector(0, 0, 74.48));
    WJ2->SetRootComponent(WJ2Mesh);

    AActor* Fire = SpawnActor<AActor>();
    UStaticMeshComponent* FireMesh = Fire->AddComponent<UStaticMeshComponent>();
    FireMesh->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Fireball.obj"));
    FireMesh->SetLocation(FVector(0, -10, 10.81));
    Fire->SetRootComponent(FireMesh);
    UPointLightComponent* FireLight = Fire->AddComponent<UPointLightComponent>();
    FireLight->SetIntensity(0.1);
    FireLight->SetRadius(54.9);
    FireLight->SetRadiusFallOff(2);
    FireLight->SetColor(FVector4(1, 0.25, 0, 1));

    AActor* Point1 = SpawnActor<AActor>();
    UPointLightComponent* Point1Light = Point1->AddComponent<UPointLightComponent>();
    Point1Light->SetLocation(FVector(0, 7.93, 6.23));
    Point1Light->SetIntensity(3);
    Point1Light->SetRadius(9.9);
    Point1Light->SetRadiusFallOff(2);
    Point1Light->SetColor(FVector4(1, 1, 1, 1));
    Point1->SetRootComponent(Point1Light);

    AActor* Point2 = SpawnActor<AActor>();
    UPointLightComponent* Point2Light = Point2->AddComponent<UPointLightComponent>();
    Point2Light->SetLocation(FVector(0, -0.26, 2.91));
    Point2Light->SetIntensity(2);
    Point2Light->SetRadius(10);
    Point2Light->SetRadiusFallOff(1);
    Point2Light->SetColor(FVector4(1.0f, 0.251f, 0.0f, 1.0f));
    Point2->SetRootComponent(Point2Light);

    AActor* Point3 = SpawnActor<AActor>();
    UPointLightComponent* Point3Light = Point3->AddComponent<UPointLightComponent>();
    Point3Light->SetLocation(FVector(0, -6.9, 2.93));
    Point3Light->SetIntensity(2);
    Point3Light->SetRadius(10);
    Point3Light->SetRadiusFallOff(1);
    Point3Light->SetColor(FVector4(1.0f, 0.251f, 0.0f, 1.0f));
    Point3->SetRootComponent(Point3Light);

    AActor* Point4 = SpawnActor<AActor>();
    UPointLightComponent* Point4Light = Point4->AddComponent<UPointLightComponent>();
    Point4Light->SetLocation(FVector(0.22, -9.33, 3.57));
    Point4Light->SetIntensity(1.5f);
    Point4Light->SetRadius(10);
    Point4Light->SetRadiusFallOff(1);
    Point4Light->SetColor(FVector4(0.635f, 1.0f, 0.419f, 1.0f));
    Point4->SetRootComponent(Point4Light);

    AActor* Point5 = SpawnActor<AActor>();
    UPointLightComponent* Point5Light = Point5->AddComponent<UPointLightComponent>();
    Point5Light->SetLocation(FVector(9.65, -8.61, 4.82));
    Point5Light->SetIntensity(1.5f);
    Point5Light->SetRadius(26.2f);
    Point5Light->SetRadiusFallOff(2);
    Point5Light->SetColor(FVector4(0.9647f, 0.5137f, 0.761f, 1.0f));
    Point5->SetRootComponent(Point5Light);

    AActor* Point6 = SpawnActor<AActor>();
    UPointLightComponent* Point6Light = Point6->AddComponent<UPointLightComponent>();
    Point6Light->SetLocation(FVector(-2.95, 2.65, 2.47));
    Point6Light->SetIntensity(2);
    Point6Light->SetRadius(10);
    Point6Light->SetRadiusFallOff(1.81f);
    Point6Light->SetColor(FVector4(1.0f, 1.0f, 1.0f, 1.0f));
    Point6->SetRootComponent(Point6Light);

    AActor* MotionBlur = SpawnActor<AActor>();
    UMotionBlurComponent* Blur = MotionBlur->AddComponent<UMotionBlurComponent>();
    Blur->MaxBlurPixels = 13.694;
    Blur->VelocityScale = 0.119;
    MotionBlur->SetRootComponent(Blur);

    AActor* Fog = SpawnActor<AActor>();
    UExponentialHeightFogComponent* FogComp = Fog->AddComponent<UExponentialHeightFogComponent>();
    FogComp->SetLocation(FVector(0, 0, 20));
    FogComp->FogDensity = 5.0f;
    FogComp->FogFalloff = 0.03f;
    FogComp->FogEndDistance= 150.f;
    FogComp->DistanceFogIntensity = 3.5f;
    FogComp->FogColor = FVector(0.0392f, 0.0627f, 0.0784f);
    Fog->SetRootComponent(FogComp);
#endif
}

void ULevel::CreateBaseObject(EWorldType worldType)
{
    if (EditorPlayer == nullptr && worldType == EWorldType::Editor)
    {
        EditorPlayer = FObjectFactory::ConstructObject<AEditorPlayer>();
    }
    if (LocalGizmo == nullptr && worldType == EWorldType::Editor)
    {
        LocalGizmo = FObjectFactory::ConstructObject<UTransformGizmo>();
    }

    if (LocalGizmo != nullptr && worldType == EWorldType::PIE)
    {
        LocalGizmo = nullptr;
    }
    if (EditorPlayer != nullptr && worldType == EWorldType::PIE)
    {
        EditorPlayer = nullptr;
    }
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