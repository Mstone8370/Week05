#include "ActorComponent.h"

#include "GameFramework/Actor.h"

#include "UObject/Casts.h"


void UActorComponent::InitializeComponent()
{
    assert(!bHasBeenInitialized);

    bHasBeenInitialized = true;
}

void UActorComponent::UninitializeComponent()
{
    assert(bHasBeenInitialized);

    bHasBeenInitialized = false;
}

void UActorComponent::BeginPlay()
{
    bHasBegunPlay = true;
}

void UActorComponent::TickComponent(float DeltaTime)
{
}

void UActorComponent::OnComponentDestroyed()
{
}

void UActorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    assert(bHasBegunPlay);

    bHasBegunPlay = false;
}

void UActorComponent::DestroyComponent(bool bPromoteChildren)
{
    if (bIsBeingDestroyed)
    {
        return;
    }

    bIsBeingDestroyed = true;

    // Owner에서 Component 제거하기
    if (AActor* MyOwner = GetOwner())
    {
        MyOwner->RemoveOwnedComponent(this);
    }

    if (bHasBegunPlay)
    {
        EndPlay(EEndPlayReason::Destroyed);
    }

    if (bHasBeenInitialized)
    {
        UninitializeComponent();
    }

    OnComponentDestroyed();

    // 나중에 ProcessPendingDestroyObjects에서 실제로 제거됨
    GUObjectArray.MarkRemoveObject(this);
}

void UActorComponent::Activate()
{
    // TODO: Tick 다시 재생
    bIsActive = true;
}

void UActorComponent::Deactivate()
{
    // TODO: Tick 멈추기
    bIsActive = false;
}

void UActorComponent::DuplicateSubObjects()
{
    Super::DuplicateSubObjects();

    // 할거없음.
}

UObject* UActorComponent::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UActorComponent>(this);

    Cast<UActorComponent>(NewObject)->DuplicateSubObjects();
    return NewObject;
}
