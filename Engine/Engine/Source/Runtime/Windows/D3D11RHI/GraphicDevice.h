#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>

#include "EngineBaseTypes.h"
#include "Container/Array.h"

#include "Core/HAL/PlatformType.h"
#include "Core/Math/Vector4.h"
#include "LevelEditor/SLevelEditor.h"


class FEditorViewportClient;

class FGraphicsDevice {

private:
    struct FBufferData
    {
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

        ID3D11Texture2D* ViewNormalBuffer = nullptr;
        ID3D11ShaderResourceView* ViewNormalBufferSRV = nullptr;
        ID3D11RenderTargetView* ViewNormalBufferRTV = nullptr;

        ID3D11Texture2D* DepthStencilBuffer = nullptr;  // 깊이/스텐실 텍스처
        ID3D11ShaderResourceView* DepthStencilSRV = nullptr;
        ID3D11DepthStencilView* DepthStencilView = nullptr;  // 깊이/스텐실 뷰

        // Process Scene (Scene 가공)
        ID3D11Texture2D* NormalizedDepthBuffer = nullptr;
        ID3D11ShaderResourceView* NormalizedDepthSRV = nullptr;
        ID3D11RenderTargetView* NormalizedDepthRTV = nullptr;

        ID3D11Texture2D* VisualizationVelocityBuffer = nullptr;
        ID3D11ShaderResourceView* VisualizationVelocityBufferSRV = nullptr;
        ID3D11RenderTargetView* VisualizationVelocityBufferRTV = nullptr;

        // Post Processing
        ID3D11Texture2D* FogBuffer = nullptr;
        ID3D11ShaderResourceView* FogSRV = nullptr;
        ID3D11RenderTargetView* FogRTV = nullptr;

        ID3D11Texture2D* MotionBlurBuffer = nullptr;
        ID3D11ShaderResourceView* MotionBlurBufferSRV = nullptr;
        ID3D11RenderTargetView* MotionBlurBufferRTV = nullptr;

        ID3D11RenderTargetView* RTVs[5];
    };

public:
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;

    FLOAT* ClearColors[5];

    ID3D11Texture2D* FinalFrameBuffer = nullptr;
    ID3D11RenderTargetView* FinalFrameBufferRTV = nullptr;


    ID3D11RasterizerState* RasterizerStateSOLID = nullptr;
    ID3D11RasterizerState* RasterizerStateWIREFRAME = nullptr;
    DXGI_SWAP_CHAIN_DESC SwapchainDesc;


    TArray<std::shared_ptr<FBufferData>> BufferDataArray;

    UINT screenWidth = 0;
    UINT screenHeight = 0;

    // Depth-Stencil 관련 변수
    ID3D11DepthStencilState* DepthStencilState = nullptr;
    ID3D11DepthStencilState* DepthStateDisable = nullptr;

    FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f }; // 화면을 초기화(clear) 할 때 사용할 색상(RGBA)
    FLOAT ZeroColor[4] = { 0.f, 0.f, 0.f, 1.f };


    void Initialize(HWND hWindow);
    void InitializeBuffer(TArray<std::shared_ptr<FEditorViewportClient>> ViewportClients);

    void CreateDeviceAndSwapChain(HWND hWindow);
    void CreateDepthStencilBuffer(const D3D11_VIEWPORT& Viewport, std::shared_ptr<FBufferData>& BufferData);
    void CreateDepthStencilState();
    void CreateRasterizerState();
    void ReleaseDeviceAndSwapChain();
    void CreateSceneBuffer(const D3D11_VIEWPORT& Viewport, std::shared_ptr<FBufferData>& BufferData);
    void CreateFrameBuffer();
    void ReleaseFrameBuffer();
    void ReleaseSceneBuffer(std::shared_ptr<FBufferData>& BufferData);

    void CreateProcessSceneBuffer(const D3D11_VIEWPORT& Viewport, std::shared_ptr<FBufferData>& BufferData);
    void ReleaseProcessSceneBuffer(std::shared_ptr<FBufferData>& BufferData);

    void CreatePostProcessBuffer(const D3D11_VIEWPORT& Viewport, std::shared_ptr<FBufferData>& BufferData);
    void ReleasePostProcessBuffer(std::shared_ptr<FBufferData>& BufferData);

    void ReleaseRasterizerState();
    void ReleaseDepthStencilBuffer(std::shared_ptr<FBufferData>& BufferData);
    void ReleaseDepthStencilResources();
    void Release();
    void SwapBuffer();
    void Prepare(std::shared_ptr<FEditorViewportClient> ActiveViewport, uint32 ViewportIndex);
    void PrepareDepthMap(uint32 ViewportIndex);
    void PrepareVisualizationVelocity(uint32 ViewportIndex);
    void PreparePostProcess();
    void PrepareFinal();
    void OnResize(HWND hWindow, SLevelEditor* LevelEditor);
    ID3D11RasterizerState* GetCurrentRasterizer() { return CurrentRasterizer; }
    void ChangeRasterizer(EViewModeIndex evi);
    void ChangeDepthStencilState(ID3D11DepthStencilState* newDetptStencil);

    void ClearAndSetRTV(ID3D11RenderTargetView* ResourceTargetView, FLOAT Color[]) const;

public:
    //uint32 GetPixelUUID(POINT pt);
    uint32 DecodeUUIDColor(FVector4 UUIDColor);

private:
    void ReleaseBuffer(ID3D11Texture2D*& Buffer);
    void ReleaseBufferRTV(ID3D11RenderTargetView*& BufferRTV);
    void ReleaseBufferSRV(ID3D11ShaderResourceView*& BufferSRV);

private:
    ID3D11RasterizerState* CurrentRasterizer = nullptr;
};

