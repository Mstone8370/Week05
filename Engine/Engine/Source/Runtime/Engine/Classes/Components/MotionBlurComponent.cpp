#include "MotionBlurComponent.h"

void UMotionBlurComponent::DuplicateSubObjects()
{
    USceneComponent::DuplicateSubObjects();
}

UObject* UMotionBlurComponent::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UMotionBlurComponent>(this);

    Cast<UMotionBlurComponent>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void UMotionBlurComponent::InitializeComponent()
{
    USceneComponent::InitializeComponent();
}

void UMotionBlurComponent::TickComponent(float DeltaTime)
{
    USceneComponent::TickComponent(DeltaTime);
}
