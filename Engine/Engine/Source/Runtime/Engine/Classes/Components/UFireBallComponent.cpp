#include "UFireBallComponent.h"

void UFireBallComponent::InitializeComponent()
{
    // 기본값 초기화
    Intensity = 30.0f;
    Radius = 10.0f;
    RadiusFallOff = 2.0f;
    Color = FVector4(.0f, 1.0f, 1.0f, 1.0f); // 기본 주황색
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