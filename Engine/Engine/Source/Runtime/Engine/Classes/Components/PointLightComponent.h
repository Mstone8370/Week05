#pragma once
#include "PrimitiveComponent.h"

class UPointLightComponent  :public USceneComponent
{
    DECLARE_CLASS(UPointLightComponent, USceneComponent)

public:
    UPointLightComponent();
    virtual ~UPointLightComponent() = default;

    void InitializeComponent() override;

    // Getter 함수들
    float GetIntensity() const;
    float GetRadius() const;
    float GetRadiusFallOff() const;
    FVector4 GetColor() const;

    // Setter 함수들
    void SetIntensity(float NewIntensity);
    void SetRadius(float NewRadius);
    void SetRadiusFallOff(float NewRadiusFallOff);
    void SetColor(const FVector4& NewColor);

private:
    float Intensity;
    float Radius;
    float RadiusFallOff;
    FVector4 Color;
};

