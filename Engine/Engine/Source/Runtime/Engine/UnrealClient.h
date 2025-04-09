#pragma once
#include "Define.h"
#include <d3d11.h>

enum class EViewScreenLocation
{
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

class FViewport
{
public:
    FViewport();
    FViewport(EViewScreenLocation _viewLocation) : ViewLocation(_viewLocation) {}

    virtual ~FViewport();

    void Initialize();

    void InitializeViewport(const DXGI_SWAP_CHAIN_DESC& SwapChainDesc);

    void ResizeViewport(const FRect& Top, const FRect& Bottom, const FRect& Left, const FRect& Right);

    void ResizeViewport(const FRect& NewRect);

private:
    D3D11_VIEWPORT Viewport;            // 뷰포트 정보
    EViewScreenLocation ViewLocation;   // 뷰포트 위치
public:
    D3D11_VIEWPORT& GetViewport() { return Viewport; }
    //void SetViewport(D3D11_VIEWPORT _viewport) { Viewport = _viewport; }
};

