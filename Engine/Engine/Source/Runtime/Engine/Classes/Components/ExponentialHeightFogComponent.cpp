#include "ExponentialHeightFogComponent.h"

void UExponentialHeightFogComponent::DuplicateSubObjects()
{
    USceneComponent::DuplicateSubObjects();
}

UObject* UExponentialHeightFogComponent::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UExponentialHeightFogComponent>(this);

    Cast<UExponentialHeightFogComponent>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void UExponentialHeightFogComponent::InitializeComponent()
{
    USceneComponent::InitializeComponent();
}

void UExponentialHeightFogComponent::TickComponent(float DeltaTime)
{
    USceneComponent::TickComponent(DeltaTime);
}
