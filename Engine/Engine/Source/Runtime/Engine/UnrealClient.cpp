#include "UnrealClient.h"

FViewport::FViewport()
{
}

FViewport::~FViewport()
{
}

void FViewport::Initialize()
{
}

void FViewport::ResizeViewport(const DXGI_SWAP_CHAIN_DESC& SwapChainDesc)
{
    float Width = static_cast<float>(SwapChainDesc.BufferDesc.Width);
    float Height = static_cast<float>(SwapChainDesc.BufferDesc.Height);
    float HalfWidth = Width * 0.5f;
    float HalfHeight = Height * 0.5f;

    switch (ViewLocation)
    {
    case EViewScreenLocation::TopLeft:
        Viewport.TopLeftX = 0.0f;
        Viewport.TopLeftY = 0.0f;
        Viewport.Width = HalfWidth;
        Viewport.Height = HalfHeight;
        break;
    case EViewScreenLocation::TopRight:
        Viewport.TopLeftX = HalfWidth;
        Viewport.TopLeftY = 0.0f;
        Viewport.Width = HalfWidth;
        Viewport.Height = HalfHeight;
        break;
    case EViewScreenLocation::BottomLeft:
        Viewport.TopLeftX = 0.0f;
        Viewport.TopLeftY = HalfHeight;
        Viewport.Width = HalfWidth;
        Viewport.Height = HalfHeight;
        break;
    case EViewScreenLocation::BottomRight:
        Viewport.TopLeftX = HalfWidth;
        Viewport.TopLeftY = HalfHeight;
        Viewport.Width = HalfWidth;
        Viewport.Height = HalfHeight;
        break;
    default:
        break;
    }

    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
}

void FViewport::ResizeViewport(const FRect& Top, const FRect& Bottom, const FRect& Left, const FRect& Right)
{
    switch (ViewLocation)
    {
    case EViewScreenLocation::TopLeft:
        Viewport.TopLeftX = Left.leftTopX;
        Viewport.TopLeftY = Top.leftTopY;
        Viewport.Width = Left.width;
        Viewport.Height = Top.height;
        break;
    case EViewScreenLocation::TopRight:
        Viewport.TopLeftX = Right.leftTopX;
        Viewport.TopLeftY = Top.leftTopY;
        Viewport.Width = Right.width;
        Viewport.Height = Top.height;
        break;
    case EViewScreenLocation::BottomLeft:
        Viewport.TopLeftX = Left.leftTopX;
        Viewport.TopLeftY = Bottom.leftTopY;
        Viewport.Width = Left.width;
        Viewport.Height = Bottom.height;
        break;
    case EViewScreenLocation::BottomRight:
        Viewport.TopLeftX = Right.leftTopX;
        Viewport.TopLeftY = Bottom.leftTopY;
        Viewport.Width = Right.width;
        Viewport.Height = Bottom.height;
        break;
    default:
        break;
    }
}

void FViewport::ResizeViewport(const FRect& NewRect)
{
    Viewport.TopLeftX = NewRect.leftTopX;
    Viewport.TopLeftY = NewRect.leftTopY;
    Viewport.Width = NewRect.width;
    Viewport.Height = NewRect.height;
}

