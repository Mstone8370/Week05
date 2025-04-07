#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>

#include "EngineBaseTypes.h"

#include "Core/HAL/PlatformType.h"
#include "Core/Math/Vector4.h"

class FGraphicsDevice {
public:
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;

    ID3D11RenderTargetView* RTVs[4];

    ID3D11Texture2D* FinalFrameBuffer = nullptr;
    ID3D11RenderTargetView* FinalFrameBufferRTV = nullptr;

    // Scene
    ID3D11Texture2D* SceneBuffer = nullptr;
    ID3D11ShaderResourceView* SceneBufferSRV = nullptr;
    ID3D11RenderTargetView* SceneBufferRTV = nullptr;

    ID3D11Texture2D* UUIDFrameBuffer = nullptr;
    ID3D11RenderTargetView* UUIDFrameBufferRTV = nullptr;

    ID3D11Texture2D* WorldPosBuffer = nullptr;
    ID3D11ShaderResourceView* WorldPosBufferSRV = nullptr;
    ID3D11RenderTargetView* WorldPosBufferRTV = nullptr;

    ID3D11Texture2D* VelocityBuffer = nullptr;
    ID3D11ShaderResourceView* VelocityBufferSRV = nullptr;
    ID3D11RenderTargetView* VelocityBufferRTV = nullptr;

    // Process Scene (Scene 가공)
    ID3D11Texture2D* DepthBuffer = nullptr;
    ID3D11ShaderResourceView* NormalizedDepthSRV = nullptr;
    ID3D11RenderTargetView* DepthRTV = nullptr;

    // Post Processing
    ID3D11Texture2D* FogBuffer = nullptr;
    ID3D11ShaderResourceView* FogSRV = nullptr;
    ID3D11RenderTargetView* FogRTV = nullptr;

    ID3D11RasterizerState* RasterizerStateSOLID = nullptr;
    ID3D11RasterizerState* RasterizerStateWIREFRAME = nullptr;
    DXGI_SWAP_CHAIN_DESC SwapchainDesc;


    UINT screenWidth = 0;
    UINT screenHeight = 0;
    // Depth-Stencil 관련 변수
    ID3D11Texture2D* DepthStencilBuffer = nullptr;  // 깊이/스텐실 텍스처
    ID3D11ShaderResourceView* DepthStencilSRV = nullptr;
    ID3D11DepthStencilView* DepthStencilView = nullptr;  // 깊이/스텐실 뷰
    ID3D11DepthStencilState* DepthStencilState = nullptr;
    FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f }; // 화면을 초기화(clear) 할 때 사용할 색상(RGBA)

    ID3D11DepthStencilState* DepthStateDisable = nullptr;

    void Initialize(HWND hWindow);
    void CreateDeviceAndSwapChain(HWND hWindow);
    void CreateDepthStencilBuffer(HWND hWindow);
    void CreateDepthStencilState();
    void CreateRasterizerState();
    void ReleaseDeviceAndSwapChain();
    void CreateFrameBuffer();
    void ReleaseFrameBuffer();

    void CreateProcessSceneBuffer();
    void ReleaseProcessSceneBuffer();

    void CreatePostProcessBuffer();
    void ReleasePostProcessBuffer();

    void ReleaseRasterizerState();
    void ReleaseDepthStencilBuffer();
    void ReleaseDepthStencilResources();
    void Release();
    void SwapBuffer();
    void Prepare();
    void Prepare(D3D11_VIEWPORT* viewport);
    void PrepareDepthMap();
    void PreparePostProcess();
    void PrepareFinal();
    void OnResize(HWND hWindow);
    ID3D11RasterizerState* GetCurrentRasterizer() { return CurrentRasterizer; }
    void ChangeRasterizer(EViewModeIndex evi);
    void ChangeDepthStencilState(ID3D11DepthStencilState* newDetptStencil);

    uint32 GetPixelUUID(POINT pt);
    uint32 DecodeUUIDColor(FVector4 UUIDColor);

private:
    void ReleaseBuffer(ID3D11Texture2D* Buffer);
    void ReleaseBufferRTV(ID3D11RenderTargetView* BufferRTV);
    void ReleaseBufferSRV(ID3D11ShaderResourceView* BufferSRV);

private:
    ID3D11RasterizerState* CurrentRasterizer = nullptr;
};

