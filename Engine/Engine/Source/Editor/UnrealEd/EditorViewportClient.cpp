#include "EditorViewportClient.h"

#include <algorithm>
#include "fstream"
#include "sstream"
#include "ostream"
#include "Math/JungleMath.h"
#include "EngineLoop.h"
#include "UnrealClient.h"
#include "Level.h"
#include "GameFramework/Actor.h"
#include "Math/MathUtility.h"

FVector FEditorViewportClient::Pivot = FVector(0.0f, 0.0f, 0.0f);
float FEditorViewportClient::OrthoSize = 10.0f;

FEditorViewportClient::FEditorViewportClient()
    : Viewport(nullptr)
    , ViewportType(LVT_Perspective)
    , ShowFlag(31)
    , ViewMode(VMI_Lit)
{

}

FEditorViewportClient::~FEditorViewportClient()
{
    Release();
}

void FEditorViewportClient::Draw(FViewport* Viewport)
{
}

void FEditorViewportClient::Initialize(int32 viewportIndex)
{

    ViewTransformPerspective.SetLocation(FVector(8.0f, 8.0f, 8.f));
    ViewTransformPerspective.SetRotation(FVector(0.0f, 45.0f, -135.0f));
    Viewport = new FViewport(static_cast<EViewScreenLocation>(viewportIndex));
    ResizeViewport(FEngineLoop::GraphicDevice.SwapchainDesc);
    ViewportIndex = viewportIndex;
}

void FEditorViewportClient::Tick(float DeltaTime)
{
    Input();
    UpdateViewMatrix();
    UpdateProjectionMatrix();
}

void FEditorViewportClient::Release()
{
    if (Viewport)
    {
        delete Viewport;
    }
}

void FEditorViewportClient::Input()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
    {
        return;
    }

    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) // VK_RBUTTON은 마우스 오른쪽 버튼을 나타냄
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        if (!bRightMouseDown)
        {
            // 마우스 오른쪽 버튼을 처음 눌렀을 때, 마우스 위치 초기화
            GetCursorPos(&lastMousePos);
            bRightMouseDown = true;
        }
        else
        {
            // 마우스 이동량 계산
            POINT currentMousePos;
            GetCursorPos(&currentMousePos);

            // 마우스 이동 차이 계산
            int32 deltaX = currentMousePos.x - lastMousePos.x;
            int32 deltaY = currentMousePos.y - lastMousePos.y;

            // Yaw(좌우 회전) 및 Pitch(상하 회전) 값 변경
            if (IsPerspective())
            {
                CameraRotateYaw(deltaX * 0.1f);  // X 이동에 따라 좌우 회전
                CameraRotatePitch(deltaY * 0.1f);  // Y 이동에 따라 상하 회전
            }
            else
            {
                PivotMoveRight(deltaX);
                PivotMoveUp(deltaY);
            }

            SetCursorPos(lastMousePos.x, lastMousePos.y);
        }
        if (GetAsyncKeyState('A') & 0x8000)
        {
            CameraMoveRight(-1.f);
        }
        if (GetAsyncKeyState('D') & 0x8000)
        {
            CameraMoveRight(1.f);
        }
        if (GetAsyncKeyState('W') & 0x8000)
        {
            CameraMoveForward(1.f);
        }
        if (GetAsyncKeyState('S') & 0x8000)
        {
            CameraMoveForward(-1.f);
        }
        if (GetAsyncKeyState('E') & 0x8000)
        {
            CameraMoveUp(1.f);
        }
        if (GetAsyncKeyState('Q') & 0x8000)
        {
            CameraMoveUp(-1.f);
        }
    }
    else
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        bRightMouseDown = false; // 마우스 오른쪽 버튼을 떼면 상태 초기화
    }

    // Focus Selected Actor
    if (GetAsyncKeyState('F') & 0x8000)
    {
        if (AActor* PickedActor = GEngineLoop.GetLevel()->GetSelectedActor())
        {
            FViewportCameraTransform& ViewTransform = ViewTransformPerspective;
            ViewTransform.SetLocation(
                // TODO: 10.0f 대신, 정점의 min, max의 거리를 구해서 하면 좋을 듯
                PickedActor->GetActorLocation() - (ViewTransform.GetForwardVector() * 10.0f)
            );
        }
    }
}
void FEditorViewportClient::ResizeViewport(const DXGI_SWAP_CHAIN_DESC& SwapChainDesc)
{
    if (Viewport)
    {
        Viewport->ResizeViewport(SwapChainDesc);
    }
    else
    {
        UE_LOG(LogLevel::Error, "Viewport is nullptr");
    }
    AspectRatio = GEngineLoop.GetAspectRatio(GEngineLoop.GraphicDevice.SwapChain);
    UpdateProjectionMatrix();
    UpdateViewMatrix();
}

void FEditorViewportClient::ResizeViewport(FRect Top, FRect Bottom, FRect Left, FRect Right)
{
    if (Viewport)
    {
        Viewport->ResizeViewport(Top, Bottom, Left, Right);
    }
    else
    {
        UE_LOG(LogLevel::Error, "Viewport is nullptr");
    }
    AspectRatio = GEngineLoop.GetAspectRatio(GEngineLoop.GraphicDevice.SwapChain);
    UpdateProjectionMatrix();
    UpdateViewMatrix();

}
bool FEditorViewportClient::IsSelected(const POINT& Point)
{
    float TopLeftX = Viewport->GetViewport().TopLeftX;
    float TopLeftY = Viewport->GetViewport().TopLeftY;
    float Width = Viewport->GetViewport().Width;
    float Height = Viewport->GetViewport().Height;

    if (Point.x >= TopLeftX && Point.x <= TopLeftX + Width && Point.y >= TopLeftY && Point.y <= TopLeftY + Height)
    {
        return true;
    }
    return false;
}

D3D11_VIEWPORT& FEditorViewportClient::GetD3DViewport()
{
    return Viewport->GetViewport();
}

void FEditorViewportClient::CameraMoveForward(float Value)
{
    if (IsPerspective())
    {
        FVector curCameraLoc = ViewTransformPerspective.GetLocation();
        curCameraLoc = curCameraLoc + ViewTransformPerspective.GetForwardVector() * GetCameraSpeedScalar() * Value;
        ViewTransformPerspective.SetLocation(curCameraLoc);
    }
    else
    {
        Pivot.X += Value * 0.1f;
    }
}

void FEditorViewportClient::CameraMoveRight(float Value)
{
    if (IsPerspective())
    {
        FVector curCameraLoc = ViewTransformPerspective.GetLocation();
        curCameraLoc = curCameraLoc + ViewTransformPerspective.GetRightVector() * GetCameraSpeedScalar() * Value;
        ViewTransformPerspective.SetLocation(curCameraLoc);
    }
    else
    {
        Pivot.Y += Value * 0.1f;
    }
}

void FEditorViewportClient::CameraMoveUp(float Value)
{
    if (IsPerspective())
    {
        FVector curCameraLoc = ViewTransformPerspective.GetLocation();
        curCameraLoc.Z = curCameraLoc.Z + GetCameraSpeedScalar() * Value;
        ViewTransformPerspective.SetLocation(curCameraLoc);
    }
    else
    {
        Pivot.Z += Value * 0.1f;
    }
}

void FEditorViewportClient::CameraRotateYaw(float Value)
{
    FVector curCameraRot = ViewTransformPerspective.GetRotation();
    curCameraRot.Z += Value ;
    ViewTransformPerspective.SetRotation(curCameraRot);
}

void FEditorViewportClient::CameraRotatePitch(float Value)
{
    FVector curCameraRot = ViewTransformPerspective.GetRotation();
    curCameraRot.Y += Value;
    curCameraRot.Y = FMath::Max(curCameraRot.Y, -89.0f);
    curCameraRot.Y = FMath::Min(curCameraRot.Y, 89.0f);
    ViewTransformPerspective.SetRotation(curCameraRot);
}

void FEditorViewportClient::PivotMoveRight(float Value)
{
    Pivot = Pivot + ViewTransformOrthographic.GetRightVector() * Value * -0.05f;
}

void FEditorViewportClient::PivotMoveUp(float Value)
{
    Pivot = Pivot + ViewTransformOrthographic.GetUpVector() * Value * 0.05f;
}

void FEditorViewportClient::UpdateViewMatrix()
{
    if (IsPerspective())
    {
        View = JungleMath::CreateViewMatrix(ViewTransformPerspective.GetLocation(),
            ViewTransformPerspective.GetLocation() + ViewTransformPerspective.GetForwardVector(),
            FVector{ 0.0f,0.0f, 1.0f });
    }
    else
    {
        UpdateOrthoCameraLoc();
        if (ViewportType == LVT_OrthoXY || ViewportType == LVT_OrthoNegativeXY)
        {
            View = JungleMath::CreateViewMatrix(ViewTransformOrthographic.GetLocation(),
                Pivot, FVector(0.0f, -1.0f, 0.0f));
        }
        else
        {
            View = JungleMath::CreateViewMatrix(ViewTransformOrthographic.GetLocation(),
                Pivot, FVector(0.0f, 0.0f, 1.0f));
        }
    }
}

void FEditorViewportClient::UpdateProjectionMatrix()
{
    if (IsPerspective())
    {
        Projection = JungleMath::CreateProjectionMatrix(
            ViewFOV * (3.141592f / 180.0f),
            GetViewport()->GetViewport().Width/ GetViewport()->GetViewport().Height,
            nearPlane,
            farPlane
        );
    }
    else
    {
        // 스왑체인의 가로세로 비율을 구합니다.
        float aspectRatio = GetViewport()->GetViewport().Width / GetViewport()->GetViewport().Height;

        // 오쏘그래픽 너비는 줌 값과 가로세로 비율에 따라 결정됩니다.
        float orthoWidth = OrthoSize * aspectRatio;
        float orthoHeight = OrthoSize;

        // 오쏘그래픽 투영 행렬 생성 (nearPlane, farPlane 은 기존 값 사용)
        Projection = JungleMath::CreateOrthoProjectionMatrix(
            orthoWidth,
            orthoHeight,
            nearPlane,
            farPlane
        );
    }
}

bool FEditorViewportClient::IsOrtho() const
{
    return !IsPerspective();
}

bool FEditorViewportClient::IsPerspective() const
{
    return (GetViewportType() == LVT_Perspective);
}

ELevelViewportType FEditorViewportClient::GetViewportType() const
{
    ELevelViewportType EffectiveViewportType = ViewportType;
    if (EffectiveViewportType == LVT_None)
    {
        EffectiveViewportType = LVT_Perspective;
    }
    //if (bUseControllingActorViewInfo)
    //{
    //    EffectiveViewportType = (ControllingActorViewInfo.ProjectionMode == ECameraProjectionMode::Perspective) ? LVT_Perspective : LVT_OrthoFreelook;
    //}
    return EffectiveViewportType;
}

void FEditorViewportClient::SetViewportType(ELevelViewportType InViewportType)
{
    ViewportType = InViewportType;
    //ApplyViewMode(GetViewMode(), IsPerspective(), EngineShowFlags);

    //// We might have changed to an orthographic viewport; if so, update any viewport links
    //UpdateLinkedOrthoViewports(true);

    //Invalidate();
}

void FEditorViewportClient::UpdateOrthoCameraLoc()
{
    switch (ViewportType)
    {
    case LVT_OrthoXY: // Top
        ViewTransformOrthographic.SetLocation(Pivot + FVector::UpVector*100000.0f);
        ViewTransformOrthographic.SetRotation(FVector(0.0f, 90.0f, -90.0f));
        break;
    case LVT_OrthoXZ: // Front
        ViewTransformOrthographic.SetLocation(Pivot + FVector::ForwardVector*100000.0f);
        ViewTransformOrthographic.SetRotation(FVector(0.0f, 0.0f, 180.0f));
        break;
    case LVT_OrthoYZ: // Left
        ViewTransformOrthographic.SetLocation(Pivot + FVector::RightVector*100000.0f);
        ViewTransformOrthographic.SetRotation(FVector(0.0f, 0.0f, 270.0f));
        break;
    case LVT_OrthoNegativeXY: // Bottom
        ViewTransformOrthographic.SetLocation(Pivot + FVector::UpVector * -1.0f*100000.0f);
        ViewTransformOrthographic.SetRotation(FVector(0.0f, -90.0f, 90.0f));
        break;
    case LVT_OrthoNegativeXZ: // Back
        ViewTransformOrthographic.SetLocation(Pivot + FVector::ForwardVector * -1.0f*100000.0f);
        ViewTransformOrthographic.SetRotation(FVector(0.0f, 0.0f, 0.0f));
        break;
    case LVT_OrthoNegativeYZ: // Right
        ViewTransformOrthographic.SetLocation(Pivot + FVector::RightVector * -1.0f*100000.0f);
        ViewTransformOrthographic.SetRotation(FVector(0.0f, 0.0f, 90.0f));
        break;
    case LVT_Perspective:
    case LVT_MAX:
    case LVT_None:
    default:
        break;
    }
}

void FEditorViewportClient::SetOthoSize(float Value)
{
    OrthoSize += Value;
    OrthoSize = FMath::Max(OrthoSize, 0.1f);
}

void FEditorViewportClient::LoadConfig(const TMap<FString, FString>& config)
{
    FString ViewportNum = std::to_string(ViewportIndex);
    CameraSpeedSetting = GetValueFromConfig(config, "CameraSpeedSetting" + ViewportNum, 1);
    CameraSpeedScalar = GetValueFromConfig(config, "CameraSpeedScalar" + ViewportNum, 1.0f);
    GridSize = GetValueFromConfig(config, "GridSize"+ ViewportNum, 10.0f);
    ViewTransformPerspective.ViewLocation.X = GetValueFromConfig(config, "PerspectiveCameraLocX" + ViewportNum, 0.0f);
    ViewTransformPerspective.ViewLocation.Y = GetValueFromConfig(config, "PerspectiveCameraLocY" + ViewportNum, 0.0f);
    ViewTransformPerspective.ViewLocation.Z = GetValueFromConfig(config, "PerspectiveCameraLocZ" + ViewportNum, 0.0f);
    ViewTransformPerspective.ViewRotation.X = GetValueFromConfig(config, "PerspectiveCameraRotX" + ViewportNum, 0.0f);
    ViewTransformPerspective.ViewRotation.Y = GetValueFromConfig(config, "PerspectiveCameraRotY" + ViewportNum, 0.0f);
    ViewTransformPerspective.ViewRotation.Z = GetValueFromConfig(config, "PerspectiveCameraRotZ" + ViewportNum, 0.0f);
    ShowFlag = GetValueFromConfig(config, "ShowFlag" + ViewportNum, 31.0f);
    ViewMode = static_cast<EViewModeIndex>(GetValueFromConfig(config, "ViewMode" + ViewportNum, 0));
    ViewportType = static_cast<ELevelViewportType>(GetValueFromConfig(config, "ViewportType" + ViewportNum, 3));
}

void FEditorViewportClient::SaveConfig(TMap<FString, FString>& config)
{
    FString ViewportNum = std::to_string(ViewportIndex);
    config["CameraSpeedSetting"+ ViewportNum] = std::to_string(CameraSpeedSetting);
    config["CameraSpeedScalar"+ ViewportNum] = std::to_string(CameraSpeedScalar);
    config["GridSize"+ ViewportNum] = std::to_string(GridSize);
    config["PerspectiveCameraLocX" + ViewportNum] = std::to_string(ViewTransformPerspective.GetLocation().X);
    config["PerspectiveCameraLocY" + ViewportNum] = std::to_string(ViewTransformPerspective.GetLocation().Y);
    config["PerspectiveCameraLocZ" + ViewportNum] = std::to_string(ViewTransformPerspective.GetLocation().Z);
    config["PerspectiveCameraRotX" + ViewportNum] = std::to_string(ViewTransformPerspective.GetRotation().X);
    config["PerspectiveCameraRotY" + ViewportNum] = std::to_string(ViewTransformPerspective.GetRotation().Y);
    config["PerspectiveCameraRotZ" + ViewportNum] = std::to_string(ViewTransformPerspective.GetRotation().Z);
    config["ShowFlag"+ ViewportNum] = std::to_string(ShowFlag);
    config["ViewMode" + ViewportNum] = std::to_string(int32(ViewMode));
    config["ViewportType" + ViewportNum] = std::to_string(int32(ViewportType));
}

TMap<FString, FString> FEditorViewportClient::ReadIniFile(const FString& filePath)
{
    TMap<FString, FString> config;
    std::ifstream file(*filePath);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '[' || line[0] == ';') continue;
        std::istringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            config[key] = value;
        }
    }
    return config;
}

void FEditorViewportClient::WriteIniFile(const FString& filePath, const TMap<FString, FString>& config)
{
    std::ofstream file(*filePath);
    for (const auto& pair : config) {
        file << *pair.Key << "=" << *pair.Value << "\n";
    }
}

void FEditorViewportClient::SetCameraSpeedScalar(float value)
{
    if (value < 0.198f)
    {
        value = 0.198f;
    }
    else if (value > 176.0f)
    {
        value = 176.0f;
    }
    CameraSpeedScalar = value;
}


FVector FViewportCameraTransform::GetForwardVector()
{
    FVector Forward = FVector(1.f, 0.f, 0.0f);
    Forward = JungleMath::FVectorRotate(Forward, ViewRotation);
    return Forward;
}

FVector FViewportCameraTransform::GetRightVector()
{
    FVector Right = FVector(0.f, 1.f, 0.0f);
	Right = JungleMath::FVectorRotate(Right, ViewRotation);
	return Right;
}

FVector FViewportCameraTransform::GetUpVector()
{
    FVector Up = FVector(0.f, 0.f, 1.0f);
    Up = JungleMath::FVectorRotate(Up, ViewRotation);
    return Up;
}

FViewportCameraTransform::FViewportCameraTransform()
{
}
