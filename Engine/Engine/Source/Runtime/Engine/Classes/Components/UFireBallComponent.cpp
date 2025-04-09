#include "UFireBallComponent.h"

UPointLightComponent::UPointLightComponent()
    : Intensity(8.f)
    , Radius(10.f)
    , RadiusFallOff(2.f)
    , Color(1.0f, 0.25f, 0.0f, 1.0f)  // 기본 주황색
{}

void UPointLightComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

float UPointLightComponent::GetIntensity() const
{
    return Intensity;
}

float UPointLightComponent::GetRadius() const
{
    return Radius;
}

float UPointLightComponent::GetRadiusFallOff() const
{
    return RadiusFallOff;
}

FVector4 UPointLightComponent::GetColor() const
{
    return Color;
}

void UPointLightComponent::SetIntensity(float NewIntensity)
{
    Intensity = NewIntensity;
}

void UPointLightComponent::SetRadius(float NewRadius)
{
    Radius = NewRadius;
}

void UPointLightComponent::SetRadiusFallOff(float NewRadiusFallOff)
{
    RadiusFallOff = NewRadiusFallOff;
}

void UPointLightComponent::SetColor(const FVector4& NewColor)
{
    Color = NewColor;
}
