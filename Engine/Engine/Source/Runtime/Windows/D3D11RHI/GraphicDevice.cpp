#include "GraphicDevice.h"
#include <wchar.h>

#include "UnrealClient.h"
#include "UnrealEd/EditorViewportClient.h"

void FGraphicsDevice::Initialize(HWND hWindow)
{
    CreateDeviceAndSwapChain(hWindow);
    CreateDepthStencilState();
    CreateRasterizerState();
    CurrentRasterizer = RasterizerStateSOLID;
}

void FGraphicsDevice::InitializeBuffer(TArray<std::shared_ptr<FEditorViewportClient>> ViewportClients)
{
    CreateFrameBuffer();
    for (const auto& ViewportClient : ViewportClients)
    {
        std::shared_ptr<FBufferData> BufferData = std::make_shared<FBufferData>();
        BufferDataArray.Add(BufferData);

        auto Viewport = ViewportClient->GetViewport()->GetViewport();

        CreateSceneBuffer(Viewport, BufferData);
        CreatePostProcessBuffer(Viewport, BufferData);
        CreateProcessSceneBuffer(Viewport, BufferData);
        CreateDepthStencilBuffer(Viewport, BufferData);
    }
}

void FGraphicsDevice::CreateDeviceAndSwapChain(HWND hWindow)
{
    // 지원하는 Direct3D 기능 레벨을 정의
    D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

    // 스왑 체인 설정 구조체 초기화
    SwapchainDesc.BufferDesc.Width = 0; // 창 크기에 맞게 자동으로 설정
    SwapchainDesc.BufferDesc.Height = 0; // 창 크기에 맞게 자동으로 설정
    SwapchainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 색상 포맷
    SwapchainDesc.SampleDesc.Count = 1; // 멀티 샘플링 비활성화
    SwapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 렌더 타겟으로 사용
    SwapchainDesc.BufferCount = 2; // 더블 버퍼링
    SwapchainDesc.OutputWindow = hWindow; // 렌더링할 창 핸들
    SwapchainDesc.Windowed = TRUE; // 창 모드
    SwapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 스왑 방식

    // 디바이스와 스왑 체인 생성
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
        featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
        &SwapchainDesc, &SwapChain, &Device, nullptr, &DeviceContext);

    if (FAILED(hr))
    {
        MessageBox(hWindow, L"CreateDeviceAndSwapChain failed!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    // 스왑 체인 정보 가져오기 (이후에 사용을 위해)
    SwapChain->GetDesc(&SwapchainDesc);
    screenWidth = SwapchainDesc.BufferDesc.Width;
    screenHeight = SwapchainDesc.BufferDesc.Height;
}

void FGraphicsDevice::CreateDepthStencilBuffer(const D3D11_VIEWPORT& Viewport, std::shared_ptr<FBufferData>& BufferData)
{
    // 깊이/스텐실 텍스처 생성
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = Viewport.Width; // 텍스처 너비 설정
    descDepth.Height = Viewport.Height; // 텍스처 높이 설정
    descDepth.MipLevels = 1; // 미맵 레벨 수 (1로 설정하여 미맵 없음)
    descDepth.ArraySize = 1; // 텍스처 배열의 크기 (1로 단일 텍스처)
    descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS; // 24비트 깊이와 8비트 스텐실을 위한 포맷
    descDepth.SampleDesc.Count = 1; // 멀티샘플링 설정 (1로 단일 샘플)
    descDepth.SampleDesc.Quality = 0; // 샘플 퀄리티 설정
    descDepth.Usage = D3D11_USAGE_DEFAULT; // 텍스처 사용 방식
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // 깊이 스텐실 뷰로 바인딩 설정
    descDepth.CPUAccessFlags = 0; // CPU 접근 방식 설정
    descDepth.MiscFlags = 0; // 기타 플래그 설정

    HRESULT hr = Device->CreateTexture2D(&descDepth, nullptr, &BufferData->DepthStencilBuffer);

    if (FAILED(hr))
    {
        //MessageBox(hWindow, L"Failed to create depth stencilBuffer!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC DepthSRVDesc = {};
    DepthSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // 깊이 텍스처 포맷에 맞춰 설정
    DepthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    DepthSRVDesc.Texture2D.MostDetailedMip = 0;
    DepthSRVDesc.Texture2D.MipLevels = 1;

    hr = Device->CreateShaderResourceView(BufferData->DepthStencilBuffer, &DepthSRVDesc, &BufferData->DepthStencilSRV);

    if (FAILED(hr)) {
        //MessageBox(hWindow, L"Failed to create depth stencil SRV!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 깊이 스텐실 포맷
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; // 뷰 타입 설정 (2D 텍스처)
    descDSV.Texture2D.MipSlice = 0; // 사용할 밉맵 슬라이스 설정

    hr = Device->CreateDepthStencilView(BufferData->DepthStencilBuffer, // Depth stencil texture
        &descDSV, // Depth stencil desc
        &BufferData->DepthStencilView);  // [out] Depth stencil view

    if (FAILED(hr))
    {
        wchar_t errorMsg[256];
        swprintf_s(errorMsg, L"Failed to create depth stencil view! HRESULT: 0x%08X", hr);
        //MessageBox(hWindow, errorMsg, L"Error", MB_ICONERROR | MB_OK);
        return;
    }
}

void FGraphicsDevice::CreateDepthStencilState()
{
    // DepthStencil 상태 설명 설정
    D3D11_DEPTH_STENCIL_DESC dsDesc;

    // Depth test parameters
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

    // Stencil test parameters
    dsDesc.StencilEnable = true;
    dsDesc.StencilReadMask = 0xFF;
    dsDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    //// DepthStencil 상태 생성
    HRESULT hr = Device->CreateDepthStencilState(&dsDesc, &DepthStencilState);
    if (FAILED(hr)) {
        // 오류 처리
        return;
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = FALSE;  // 깊이 테스트 유지
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;  // 깊이 버퍼에 쓰지 않음
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;  // 깊이 비교를 항상 통과
    Device->CreateDepthStencilState(&depthStencilDesc, &DepthStateDisable);

}

void FGraphicsDevice::CreateRasterizerState()
{
    D3D11_RASTERIZER_DESC rasterizerdesc = {};
    rasterizerdesc.FillMode = D3D11_FILL_SOLID;
    rasterizerdesc.CullMode = D3D11_CULL_BACK;
    rasterizerdesc.ScissorEnable = false;
    Device->CreateRasterizerState(&rasterizerdesc, &RasterizerStateSOLID);

    rasterizerdesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterizerdesc.CullMode = D3D11_CULL_BACK;
    rasterizerdesc.ScissorEnable = false;
    Device->CreateRasterizerState(&rasterizerdesc, &RasterizerStateWIREFRAME);
}

void FGraphicsDevice::ReleaseDeviceAndSwapChain()
{
    if (DeviceContext)
    {
        DeviceContext->Flush(); // 남아있는 GPU 명령 실행
    }

    if (SwapChain)
    {
        SwapChain->Release();
        SwapChain = nullptr;
    }

    if (Device)
    {
        Device->Release();
        Device = nullptr;
    }

    if (DeviceContext)
    {
        DeviceContext->Release();
        DeviceContext = nullptr;
    }
}

void FGraphicsDevice::CreateSceneBuffer(const D3D11_VIEWPORT& Viewport, std::shared_ptr<FBufferData>& BufferData)
{
    D3D11_TEXTURE2D_DESC SceneSrvTextureDesc = {};
    SceneSrvTextureDesc.Width = Viewport.Width;
    SceneSrvTextureDesc.Height = Viewport.Height;
    SceneSrvTextureDesc.MipLevels = 1;
    SceneSrvTextureDesc.ArraySize = 1;
    SceneSrvTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SceneSrvTextureDesc.SampleDesc.Count = 1;
    SceneSrvTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    SceneSrvTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    Device->CreateTexture2D(&SceneSrvTextureDesc, nullptr, &BufferData->SceneBuffer);

    // TODO Check - Format을 이전꺼랑 동일하게 했어야 되는지 헷갈림, 일단 동일하게 함.

    D3D11_SHADER_RESOURCE_VIEW_DESC SceneSRVDEsc = {};
    SceneSRVDEsc.Format = SceneSrvTextureDesc.Format;
    SceneSRVDEsc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SceneSRVDEsc.Texture2D.MostDetailedMip = 0;
    SceneSRVDEsc.Texture2D.MipLevels = 1;

    Device->CreateShaderResourceView(BufferData->SceneBuffer, &SceneSRVDEsc, &BufferData->SceneBufferSRV);

    // 렌더 타겟 뷰 생성
    D3D11_RENDER_TARGET_VIEW_DESC SceneRTVDesc = {};
    SceneRTVDesc.Format = SceneSrvTextureDesc.Format; // 색상 포맷
    SceneRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

    Device->CreateRenderTargetView(BufferData->SceneBuffer, &SceneRTVDesc, &BufferData->SceneBufferRTV);

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = Viewport.Width;
    textureDesc.Height = Viewport.Height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    Device->CreateTexture2D(&textureDesc, nullptr, &BufferData->UUIDFrameBuffer);

    D3D11_RENDER_TARGET_VIEW_DESC UUIDFrameBufferRTVDesc = {};
    UUIDFrameBufferRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      // 색상 포맷
    UUIDFrameBufferRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

    Device->CreateRenderTargetView(BufferData->UUIDFrameBuffer, &UUIDFrameBufferRTVDesc, &BufferData->UUIDFrameBufferRTV);

    // WorldPos
    {
        D3D11_TEXTURE2D_DESC BufferDesc = {};
        BufferDesc.Width = Viewport.Width;
        BufferDesc.Height = Viewport.Height;
        BufferDesc.MipLevels = 1;
        BufferDesc.ArraySize = 1;
        BufferDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
        BufferDesc.SampleDesc.Count = 1;
        BufferDesc.Usage = D3D11_USAGE_DEFAULT;
        BufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        auto hr = Device->CreateTexture2D(&BufferDesc, nullptr, &BufferData->WorldPosBuffer);

        // TODO Check - Format을 이전꺼랑 동일하게 했어야 되는지 헷갈림, 일단 동일하게 함.

        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = BufferDesc.Format;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MostDetailedMip = 0;
        SRVDesc.Texture2D.MipLevels = 1;

        hr = Device->CreateShaderResourceView(BufferData->WorldPosBuffer, &SRVDesc, &BufferData->WorldPosBufferSRV);

        // 렌더 타겟 뷰 생성
        D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
        RTVDesc.Format = BufferDesc.Format; // 색상 포맷
        RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

        hr = Device->CreateRenderTargetView(BufferData->WorldPosBuffer, &RTVDesc, &BufferData->WorldPosBufferRTV);
    }

    // Velocity
    {
        D3D11_TEXTURE2D_DESC BufferDesc = {};
        BufferDesc.Width = Viewport.Width;
        BufferDesc.Height = Viewport.Height;
        BufferDesc.MipLevels = 1;
        BufferDesc.ArraySize = 1;
        BufferDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
        BufferDesc.SampleDesc.Count = 1;
        BufferDesc.Usage = D3D11_USAGE_DEFAULT;
        BufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        auto hr = Device->CreateTexture2D(&BufferDesc, nullptr, &BufferData->VelocityBuffer);

        // TODO Check - Format을 이전꺼랑 동일하게 했어야 되는지 헷갈림, 일단 동일하게 함.

        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = BufferDesc.Format;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MostDetailedMip = 0;
        SRVDesc.Texture2D.MipLevels = 1;

        hr = Device->CreateShaderResourceView(BufferData->VelocityBuffer, &SRVDesc, &BufferData->VelocityBufferSRV);

        // 렌더 타겟 뷰 생성
        D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
        RTVDesc.Format = BufferDesc.Format; // 색상 포맷
        RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

        hr = Device->CreateRenderTargetView(BufferData->VelocityBuffer, &RTVDesc, &BufferData->VelocityBufferRTV);
    }

    // ViewNormal
    {
        D3D11_TEXTURE2D_DESC BufferDesc = {};
        BufferDesc.Width = Viewport.Width;
        BufferDesc.Height = Viewport.Height;
        BufferDesc.MipLevels = 1;
        BufferDesc.ArraySize = 1;
        BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        BufferDesc.SampleDesc.Count = 1;
        BufferDesc.Usage = D3D11_USAGE_DEFAULT;
        BufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        auto hr = Device->CreateTexture2D(&BufferDesc, nullptr, &BufferData->ViewNormalBuffer);

        // TODO Check - Format을 이전꺼랑 동일하게 했어야 되는지 헷갈림, 일단 동일하게 함.

        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = BufferDesc.Format;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MostDetailedMip = 0;
        SRVDesc.Texture2D.MipLevels = 1;

        hr = Device->CreateShaderResourceView(BufferData->ViewNormalBuffer, &SRVDesc, &BufferData->ViewNormalBufferSRV);

        // 렌더 타겟 뷰 생성
        D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
        RTVDesc.Format = BufferDesc.Format; // 색상 포맷
        RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

        hr = Device->CreateRenderTargetView(BufferData->ViewNormalBuffer, &RTVDesc, &BufferData->ViewNormalBufferRTV);
    }


    BufferData->RTVs[0] = BufferData->SceneBufferRTV;
    BufferData->RTVs[1] = BufferData->UUIDFrameBufferRTV;
    BufferData->RTVs[2] = BufferData->WorldPosBufferRTV;
    BufferData->RTVs[3] = BufferData->VelocityBufferRTV;
    BufferData->RTVs[4] = BufferData->ViewNormalBufferRTV;

    ClearColors[0] = ClearColor;
    ClearColors[1] = ZeroColor;
    ClearColors[2] = ZeroColor;
    ClearColors[3] = ZeroColor;
    ClearColors[4] = ZeroColor;
}

void FGraphicsDevice::CreateFrameBuffer()
{
    // 스왑 체인으로부터 백 버퍼 텍스처 가져오기
    SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FinalFrameBuffer);

    // 렌더 타겟 뷰 생성
    D3D11_RENDER_TARGET_VIEW_DESC FinalFrameRTVDesc = {};
    FinalFrameRTVDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // 색상 포맷
    FinalFrameRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

    Device->CreateRenderTargetView(FinalFrameBuffer, &FinalFrameRTVDesc, &FinalFrameBufferRTV);
}

void FGraphicsDevice::ReleaseFrameBuffer()
{
    ReleaseBuffer(FinalFrameBuffer);
    ReleaseBufferRTV(FinalFrameBufferRTV);
}

void FGraphicsDevice::ReleaseSceneBuffer(std::shared_ptr<FBufferData>& BufferData)
{
    ReleaseBuffer(BufferData->SceneBuffer);
    ReleaseBufferRTV(BufferData->SceneBufferRTV);
    ReleaseBufferSRV(BufferData->SceneBufferSRV);

    ReleaseBuffer(BufferData->UUIDFrameBuffer);
    ReleaseBufferRTV(BufferData->UUIDFrameBufferRTV);

    ReleaseBuffer(BufferData->WorldPosBuffer);
    ReleaseBufferRTV(BufferData->WorldPosBufferRTV);
    ReleaseBufferSRV(BufferData->WorldPosBufferSRV);

    ReleaseBuffer(BufferData->VelocityBuffer);
    ReleaseBufferRTV(BufferData->VelocityBufferRTV);
    ReleaseBufferSRV(BufferData->VelocityBufferSRV);

    ReleaseBuffer(BufferData->ViewNormalBuffer);
    ReleaseBufferRTV(BufferData->ViewNormalBufferRTV);
    ReleaseBufferSRV(BufferData->ViewNormalBufferSRV);

    for (auto*& RTV : BufferData->RTVs)
    {
        RTV = nullptr;
    }
}

void FGraphicsDevice::CreateProcessSceneBuffer(const D3D11_VIEWPORT& Viewport, std::shared_ptr<FBufferData>& BufferData)
{
    // Depth - Normalize
    {
        D3D11_TEXTURE2D_DESC BufferDesc = {};
        BufferDesc.Width = Viewport.Width;
        BufferDesc.Height = Viewport.Height;
        BufferDesc.MipLevels = 1;
        BufferDesc.ArraySize = 1;
        BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        BufferDesc.SampleDesc.Count = 1;
        BufferDesc.Usage = D3D11_USAGE_DEFAULT;
        BufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        Device->CreateTexture2D(&BufferDesc, nullptr, &BufferData->NormalizedDepthBuffer);

        // TODO Check - Format을 이전꺼랑 동일하게 했어야 되는지 헷갈림, 일단 동일하게 함.

        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = BufferDesc.Format;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MostDetailedMip = 0;
        SRVDesc.Texture2D.MipLevels = 1;

        Device->CreateShaderResourceView(BufferData->NormalizedDepthBuffer, &SRVDesc, &BufferData->NormalizedDepthSRV);

        // 렌더 타겟 뷰 생성
        D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
        RTVDesc.Format = BufferDesc.Format; // 색상 포맷
        RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

        Device->CreateRenderTargetView(BufferData->NormalizedDepthBuffer, &RTVDesc, &BufferData->NormalizedDepthRTV);
    }

    // Velocity - Visualization
    {
        D3D11_TEXTURE2D_DESC BufferDesc = {};
        BufferDesc.Width = Viewport.Width;
        BufferDesc.Height = Viewport.Height;
        BufferDesc.MipLevels = 1;
        BufferDesc.ArraySize = 1;
        BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        BufferDesc.SampleDesc.Count = 1;
        BufferDesc.Usage = D3D11_USAGE_DEFAULT;
        BufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        Device->CreateTexture2D(&BufferDesc, nullptr, &BufferData->VisualizationVelocityBuffer);

        // TODO Check - Format을 이전꺼랑 동일하게 했어야 되는지 헷갈림, 일단 동일하게 함.

        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = BufferDesc.Format;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MostDetailedMip = 0;
        SRVDesc.Texture2D.MipLevels = 1;

        Device->CreateShaderResourceView(BufferData->VisualizationVelocityBuffer, &SRVDesc, &BufferData->VisualizationVelocityBufferSRV);

        // 렌더 타겟 뷰 생성
        D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
        RTVDesc.Format = BufferDesc.Format; // 색상 포맷
        RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

        Device->CreateRenderTargetView(BufferData->VisualizationVelocityBuffer, &RTVDesc, &BufferData->VisualizationVelocityBufferRTV);
    }
}

void FGraphicsDevice::ReleaseProcessSceneBuffer(std::shared_ptr<FBufferData>& BufferData)
{
    ReleaseBuffer(BufferData->NormalizedDepthBuffer);
    ReleaseBufferSRV(BufferData->NormalizedDepthSRV);
    ReleaseBufferRTV(BufferData->NormalizedDepthRTV);

    ReleaseBuffer(BufferData->VisualizationVelocityBuffer);
    ReleaseBufferSRV(BufferData->VisualizationVelocityBufferSRV);
    ReleaseBufferRTV(BufferData->VisualizationVelocityBufferRTV);
}

void FGraphicsDevice::CreatePostProcessBuffer(const D3D11_VIEWPORT& Viewport, std::shared_ptr<FBufferData>& BufferData)
{
    // Fog
    {
        D3D11_TEXTURE2D_DESC FogSrvTextureDesc = {};
        FogSrvTextureDesc.Width = Viewport.Width;
        FogSrvTextureDesc.Height = Viewport.Height;
        FogSrvTextureDesc.MipLevels = 1;
        FogSrvTextureDesc.ArraySize = 1;
        FogSrvTextureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        FogSrvTextureDesc.SampleDesc.Count = 1;
        FogSrvTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        FogSrvTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        Device->CreateTexture2D(&FogSrvTextureDesc, nullptr, &BufferData->FogBuffer);

        // TODO Check - Format을 이전꺼랑 동일하게 했어야 되는지 헷갈림, 일단 동일하게 함.

        D3D11_SHADER_RESOURCE_VIEW_DESC FogSRVDEsc = {};
        FogSRVDEsc.Format = FogSrvTextureDesc.Format;
        FogSRVDEsc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        FogSRVDEsc.Texture2D.MostDetailedMip = 0;
        FogSRVDEsc.Texture2D.MipLevels = 1;

        Device->CreateShaderResourceView(BufferData->FogBuffer, &FogSRVDEsc, &BufferData->FogSRV);

        // 렌더 타겟 뷰 생성
        D3D11_RENDER_TARGET_VIEW_DESC FogRTVDesc = {};
        FogRTVDesc.Format = FogSrvTextureDesc.Format; // 색상 포맷
        FogRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

        Device->CreateRenderTargetView(BufferData->FogBuffer, &FogRTVDesc, &BufferData->FogRTV);
    }

    // Motion Blur
    {
        D3D11_TEXTURE2D_DESC BufferDesc = {};
        BufferDesc.Width = Viewport.Width;
        BufferDesc.Height = Viewport.Height;
        BufferDesc.MipLevels = 1;
        BufferDesc.ArraySize = 1;
        BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        BufferDesc.SampleDesc.Count = 1;
        BufferDesc.Usage = D3D11_USAGE_DEFAULT;
        BufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        Device->CreateTexture2D(&BufferDesc, nullptr, &BufferData->MotionBlurBuffer);

        // TODO Check - Format을 이전꺼랑 동일하게 했어야 되는지 헷갈림, 일단 동일하게 함.

        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDEsc = {};
        SRVDEsc.Format = BufferDesc.Format;
        SRVDEsc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        SRVDEsc.Texture2D.MostDetailedMip = 0;
        SRVDEsc.Texture2D.MipLevels = 1;

        Device->CreateShaderResourceView(BufferData->MotionBlurBuffer, &SRVDEsc, &BufferData->MotionBlurBufferSRV);

        // 렌더 타겟 뷰 생성
        D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
        RTVDesc.Format = BufferDesc.Format; // 색상 포맷
        RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

        Device->CreateRenderTargetView(BufferData->MotionBlurBuffer, &RTVDesc, &BufferData->MotionBlurBufferRTV);
    }
}

void FGraphicsDevice::ReleasePostProcessBuffer(std::shared_ptr<FBufferData>& BufferData)
{
    ReleaseBufferSRV(BufferData->FogSRV);
    ReleaseBuffer(BufferData->FogBuffer);
    ReleaseBufferRTV(BufferData->FogRTV);

    ReleaseBuffer(BufferData->MotionBlurBuffer);
    ReleaseBufferSRV(BufferData->MotionBlurBufferSRV);
    ReleaseBufferRTV(BufferData->MotionBlurBufferRTV);
}

void FGraphicsDevice::ReleaseRasterizerState()
{
    if (RasterizerStateSOLID)
    {
        RasterizerStateSOLID->Release();
        RasterizerStateSOLID = nullptr;
    }
    if (RasterizerStateWIREFRAME)
    {
        RasterizerStateWIREFRAME->Release();
        RasterizerStateWIREFRAME = nullptr;
    }
}

void FGraphicsDevice::ReleaseDepthStencilBuffer(std::shared_ptr<FBufferData>& BufferData)
{
    if (BufferData->DepthStencilView)
    {
        BufferData->DepthStencilView->Release();
        BufferData->DepthStencilView = nullptr;
    }

    ReleaseBuffer(BufferData->DepthStencilBuffer);

    ReleaseBufferSRV(BufferData->DepthStencilSRV);
}

void FGraphicsDevice::ReleaseDepthStencilResources()
{
    // 깊이/스텐실 상태 해제
    if (DepthStencilState) {
        DepthStencilState->Release();
        DepthStencilState = nullptr;
    }
    if (DepthStateDisable) {
        DepthStateDisable->Release();
        DepthStateDisable = nullptr;
    }
}

void FGraphicsDevice::Release()
{
    ReleaseRasterizerState();
    DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ReleaseFrameBuffer();
    for (auto& BufferData : BufferDataArray)
    {
        ReleaseSceneBuffer(BufferData);
        ReleasePostProcessBuffer(BufferData);
        ReleaseProcessSceneBuffer(BufferData);
        ReleaseDepthStencilBuffer(BufferData);
        ReleaseDepthStencilResources();
        ReleaseDeviceAndSwapChain();
    }
}

void FGraphicsDevice::SwapBuffer()
{
    SwapChain->Present(1, 0);
}

void FGraphicsDevice::Prepare(std::shared_ptr<FEditorViewportClient> ActiveViewport, uint32 ViewportIndex)
{
    for (uint64 i = 0; i < std::size(BufferDataArray[ViewportIndex]->RTVs); i++)
    {
        DeviceContext->ClearRenderTargetView(BufferDataArray[ViewportIndex]->RTVs[i], ClearColors[i]); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    }
    DeviceContext->ClearDepthStencilView(BufferDataArray[ViewportIndex]->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0); // 깊이 버퍼 초기화 추가

    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정

    // DeviceContext->RSSetViewports(1, &ViewportInfo); // GPU가 화면을 렌더링할 영역 설정
    DeviceContext->RSSetState(CurrentRasterizer); //레스터 라이저 상태 설정

    DeviceContext->RSSetViewports(1, &ActiveViewport->GetViewport()->GetViewport()); // GPU가 화면을 렌더링할 영역 설정

    DeviceContext->OMSetDepthStencilState(DepthStencilState, 0);

    DeviceContext->OMSetRenderTargets(std::size(BufferDataArray[ViewportIndex]->RTVs), BufferDataArray[ViewportIndex]->RTVs, BufferDataArray[ViewportIndex]->DepthStencilView); // 렌더 타겟 설정(백버퍼를 가르킴)
    DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
}

void FGraphicsDevice::PrepareDepthMap(uint32 ViewportIndex)
{
    DeviceContext->ClearRenderTargetView(BufferDataArray[ViewportIndex]->NormalizedDepthRTV, ClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정

    ChangeRasterizer(EViewModeIndex::VMI_Lit);

    DeviceContext->OMSetRenderTargets(1, &BufferDataArray[ViewportIndex]->NormalizedDepthRTV, nullptr);
    DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
}

void FGraphicsDevice::PrepareVisualizationVelocity(uint32 ViewportIndex)
{
    DeviceContext->ClearRenderTargetView(BufferDataArray[ViewportIndex]->VisualizationVelocityBufferRTV, ZeroColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정

    ChangeRasterizer(EViewModeIndex::VMI_Lit);

    DeviceContext->OMSetRenderTargets(1, &BufferDataArray[ViewportIndex]->VisualizationVelocityBufferRTV, nullptr);
    DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
}

void FGraphicsDevice::PreparePostProcess()
{
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정

    ChangeRasterizer(EViewModeIndex::VMI_Lit);

    DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
}

void FGraphicsDevice::PrepareFinal()
{
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정

    ChangeRasterizer(EViewModeIndex::VMI_Lit);

    DeviceContext->OMSetRenderTargets(1, &FinalFrameBufferRTV, nullptr);
    DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
}


void FGraphicsDevice::OnResize(HWND hWindow, SLevelEditor* LevelEditor) {
    if (SwapChain == nullptr)
    {
        return;
    }

    // DeviceContext->OMSetDepthStencilState(nullptr, 0);
    // DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
    // DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ReleaseFrameBuffer();

    for (auto& BufferData : BufferDataArray)
    {
        ReleaseSceneBuffer(BufferData);
        ReleasePostProcessBuffer(BufferData);
        ReleaseProcessSceneBuffer(BufferData);
        ReleaseDepthStencilBuffer(BufferData);
    }
    BufferDataArray.Empty();

    // int n = 10;
    // for (int i = 0; i < n; i++)
    // {
    //     ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
    //     DeviceContext->PSSetShaderResources(i, 1, nullSRV);
    //     DeviceContext->VSSetShaderResources(0, 1, nullSRV);
    //     DeviceContext->CSSetShaderResources(0, 1, nullSRV);
    // }


    if (screenWidth == 0 || screenHeight == 0) {
        MessageBox(hWindow, L"Invalid width or height for ResizeBuffers!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    // SwapChain 크기 조정
    HRESULT hr;
    hr = SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0);  // DXGI_FORMAT_B8G8R8A8_UNORM으로 시도
    if (FAILED(hr)) {
        MessageBox(hWindow, L"failed", L"ResizeBuffers failed ", MB_ICONERROR | MB_OK);
        return;
    }

    SwapChain->GetDesc(&SwapchainDesc);
    screenWidth = SwapchainDesc.BufferDesc.Width;
    screenHeight = SwapchainDesc.BufferDesc.Height;

    LevelEditor->ResizeLevel(screenWidth, screenHeight);

    CreateFrameBuffer();
    for (const auto& ViewportClient : LevelEditor->GetViewports())
    {
        std::shared_ptr<FBufferData> BufferData = std::make_shared<FBufferData>();
        BufferDataArray.Add(BufferData);

        auto Viewport = ViewportClient->GetViewport()->GetViewport();

        CreateSceneBuffer(Viewport, BufferData);
        CreatePostProcessBuffer(Viewport, BufferData);
        CreateProcessSceneBuffer(Viewport, BufferData);
        CreateDepthStencilBuffer(Viewport, BufferData);
    }
}


void FGraphicsDevice::ChangeRasterizer(EViewModeIndex evi)
{
    switch (evi)
    {
    case EViewModeIndex::VMI_Wireframe:
        CurrentRasterizer = RasterizerStateWIREFRAME;
        break;
    case EViewModeIndex::VMI_Lit:
    case EViewModeIndex::VMI_Unlit:
    case EViewModeIndex::VMI_SceneDepth:
    case EViewModeIndex::VMI_Velocity:
    case EViewModeIndex::VMI_Normal:
        CurrentRasterizer = RasterizerStateSOLID;
        break;
    default:
        break;
    }
    DeviceContext->RSSetState(CurrentRasterizer); //레스터 라이저 상태 설정
}

void FGraphicsDevice::ChangeDepthStencilState(ID3D11DepthStencilState* newDetptStencil)
{
    DeviceContext->OMSetDepthStencilState(newDetptStencil, 0);
}

void FGraphicsDevice::ClearAndSetRTV(ID3D11RenderTargetView* ResourceTargetView, FLOAT Color[]) const
{
    DeviceContext->ClearRenderTargetView(ResourceTargetView, Color); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->OMSetRenderTargets(1, &ResourceTargetView, nullptr);
}

// uint32 FGraphicsDevice::GetPixelUUID(POINT pt)
// {
//     // pt.x 값 제한하기
//     if (pt.x < 0) {
//         pt.x = 0;
//     }
//     else if (pt.x > screenWidth) {
//         pt.x = screenWidth;
//     }
//
//     // pt.y 값 제한하기
//     if (pt.y < 0) {
//         pt.y = 0;
//     }
//     else if (pt.y > screenHeight) {
//         pt.y = screenHeight;
//     }
//
//     // 1. Staging 텍스처 생성 (1x1 픽셀)
//     D3D11_TEXTURE2D_DESC stagingDesc = {};
//     stagingDesc.Width = 1; // 픽셀 1개만 복사
//     stagingDesc.Height = 1;
//     stagingDesc.MipLevels = 1;
//     stagingDesc.ArraySize = 1;
//     stagingDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 원본 텍스처 포맷과 동일
//     stagingDesc.SampleDesc.Count = 1;
//     stagingDesc.Usage = D3D11_USAGE_STAGING;
//     stagingDesc.BindFlags = 0;
//     stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//
//     ID3D11Texture2D* stagingTexture = nullptr;
//     Device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
//
//     // 2. 복사할 영역 정의 (D3D11_BOX)
//     D3D11_BOX srcBox = {};
//     srcBox.left = static_cast<UINT>(pt.x);
//     srcBox.right = srcBox.left + 1; // 1픽셀 너비
//     srcBox.top = static_cast<UINT>(pt.y);
//     srcBox.bottom = srcBox.top + 1; // 1픽셀 높이
//     srcBox.front = 0;
//     srcBox.back = 1;
//     FVector4 UUIDColor{ 1, 1, 1, 1 };
//
//     if (stagingTexture == nullptr)
//         return DecodeUUIDColor(UUIDColor);
//
//     // 3. 특정 좌표만 복사
//    DeviceContext->CopySubresourceRegion(
//         stagingTexture, // 대상 텍스처
//         0,              // 대상 서브리소스
//         0, 0, 0,        // 대상 좌표 (x, y, z)
//         UUIDFrameBuffer, // 원본 텍스처
//         0,              // 원본 서브리소스
//         &srcBox         // 복사 영역
//     );
//
//     // 4. 데이터 매핑
//     D3D11_MAPPED_SUBRESOURCE mapped = {};
//     DeviceContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapped);
//
//     // 5. 픽셀 데이터 추출 (1x1 텍스처이므로 offset = 0)
//     const BYTE* pixelData = static_cast<const BYTE*>(mapped.pData);
//
//     if (pixelData)
//     {
//         UUIDColor.x = static_cast<float>(pixelData[0]); // R
//         UUIDColor.y = static_cast<float>(pixelData[1]); // G
//         UUIDColor.z = static_cast<float>(pixelData[2]) ; // B
//         UUIDColor.a = static_cast<float>(pixelData[3]); // A
//     }
//
//     // 6. 매핑 해제 및 정리
//     DeviceContext->Unmap(stagingTexture, 0);
//     if (stagingTexture) stagingTexture->Release(); stagingTexture = nullptr;
//
//     return DecodeUUIDColor(UUIDColor);
// }

uint32 FGraphicsDevice::DecodeUUIDColor(FVector4 UUIDColor) {
    uint32_t W = static_cast<uint32_t>(UUIDColor.a) << 24;
    uint32_t Z = static_cast<uint32_t>(UUIDColor.z) << 16;
    uint32_t Y = static_cast<uint32_t>(UUIDColor.y) << 8;
    uint32_t X = static_cast<uint32_t>(UUIDColor.x);

    return W | Z | Y | X;
}

void FGraphicsDevice::ReleaseBuffer(ID3D11Texture2D*& Buffer)
{
    if (Buffer)
    {
        Buffer->Release();
        Buffer = nullptr;
    }
}

void FGraphicsDevice::ReleaseBufferRTV(ID3D11RenderTargetView*& BufferRTV)
{
    if (BufferRTV)
    {
        BufferRTV->Release();
        BufferRTV = nullptr;
    }
}

void FGraphicsDevice::ReleaseBufferSRV(ID3D11ShaderResourceView*& BufferSRV)
{
    if (BufferSRV)
    {
        BufferSRV->Release();
        BufferSRV = nullptr;
    }
}