#pragma once
#include "SceneComponent.h"

class UExponentialHeightFogComponent : public USceneComponent
{
    DECLARE_CLASS(UExponentialHeightFogComponent, USceneComponent);

public:
    UExponentialHeightFogComponent() = default;
    virtual ~UExponentialHeightFogComponent() override = default;

    virtual void DuplicateSubObjects() override;
    virtual UObject* Duplicate() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

public:
    FVector FogColor = FVector(0.5, 0.6, 0.7);
    float FogDensity = 0.5f;
    float FogFalloff = 0.1f;
    float FogHeight = 0.2f;
    float FogStartDistance = 50.f;
    float FogEndDistance = 1000.f;
    float DistanceFogIntensity = 0.5f;
};
