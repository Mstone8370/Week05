#pragma once
#include "SceneComponent.h"

class UMotionBlurComponent : public USceneComponent
{
    DECLARE_CLASS(UMotionBlurComponent, USceneComponent)

public:
    UMotionBlurComponent() = default;
    virtual ~UMotionBlurComponent() override = default;

    virtual void DuplicateSubObjects() override;
    virtual UObject* Duplicate() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

public:
    bool   bIsActive = true;
    float  MaxBlurPixels;    // 최대 블러 거리 (픽셀 단위)
    float  VelocityScale = 0.01f;
    float  DepthThreshold = 0.01f;
};
