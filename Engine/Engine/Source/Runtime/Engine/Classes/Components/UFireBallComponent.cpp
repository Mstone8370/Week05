#include "UFireBallComponent.h"

UFireBallComponent::UFireBallComponent()
    : Intensity(30.f)
    , Radius(10.f)
    , RadiusFallOff(2.f)
    , Color(1.0f, 0.25f, 0.0f, 1.0f)  // 기본 주황색
{}

void UFireBallComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

float UFireBallComponent::GetIntensity() const
{
    return Intensity;
}

float UFireBallComponent::GetRadius() const
{
    return Radius;
}

float UFireBallComponent::GetRadiusFallOff() const
{
    return RadiusFallOff;
}

FVector4 UFireBallComponent::GetColor() const
{
    return Color;
}

void UFireBallComponent::SetIntensity(float NewIntensity)
{
    Intensity = NewIntensity;
}

void UFireBallComponent::SetRadius(float NewRadius)
{
    Radius = NewRadius;
}

void UFireBallComponent::SetRadiusFallOff(float NewRadiusFallOff)
{
    RadiusFallOff = NewRadiusFallOff;
}

void UFireBallComponent::SetColor(const FVector4& NewColor)
{
    Color = NewColor;
}
