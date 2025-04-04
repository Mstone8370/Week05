#pragma once

#include <DirectXMath.h>

struct FVector2D
{
	float X,Y;
	FVector2D(float _x = 0, float _y = 0) : X(_x), Y(_y) {}

	FVector2D operator+(const FVector2D& rhs) const
	{
		return FVector2D(X + rhs.X, Y + rhs.Y);
	}
	FVector2D operator-(const FVector2D& rhs) const
	{
		return FVector2D(X - rhs.X, Y - rhs.Y);
	}
	FVector2D operator*(float rhs) const
	{
		return FVector2D(X * rhs, Y * rhs);
	}
	FVector2D operator/(float rhs) const
	{
		return FVector2D(X / rhs, Y / rhs);
	}
	FVector2D& operator+=(const FVector2D& rhs)
	{
		X += rhs.X;
		Y += rhs.Y;
		return *this;
	}
};

// 3D 벡터
struct FVector
{
    float X, Y, Z;
    FVector(float _x = 0, float _y = 0, float _z = 0) : X(_x), Y(_y), Z(_z) {}

    FVector operator-(const FVector& other) const {
        return FVector(X - other.X, Y - other.Y, Z - other.Z);
    }
    FVector operator+(const FVector& other) const {
        return FVector(X + other.X, Y + other.Y, Z + other.Z);
    }

    // 벡터 내적
    float Dot(const FVector& other) const {
        return X * other.X + Y * other.Y + Z * other.Z;
    }

    // 벡터 크기
    float Magnitude() const {
        return sqrt(X * X + Y * Y + Z * Z);
    }

    // 벡터 정규화
    FVector Normalize() const {
        float mag = Magnitude();
        return (mag > 0) ? FVector(X / mag, Y / mag, Z / mag) : FVector(0, 0, 0);
    }
    FVector Cross(const FVector& Other) const
    {
        return FVector{
            Y * Other.Z - Z * Other.Y,
            Z * Other.X - X * Other.Z,
            X * Other.Y - Y * Other.X
        };
    }
    // 스칼라 곱셈
    FVector operator*(float scalar) const {
        return FVector(X * scalar, Y * scalar, Z * scalar);
    }

    bool operator==(const FVector& other) const {
        return (X == other.X && Y == other.Y && Z == other.Z);
    }

    float Distance(const FVector& other) const {
        // 두 벡터의 차 벡터의 크기를 계산
        return ((*this - other).Magnitude());
    }
    DirectX::XMFLOAT3 ToXMFLOAT3() const
    {
        return DirectX::XMFLOAT3(X, Y, Z);
    }

    static const FVector ZeroVector;
    static const FVector OneVector;
    static const FVector UpVector;
    static const FVector ForwardVector;
    static const FVector RightVector;
};
