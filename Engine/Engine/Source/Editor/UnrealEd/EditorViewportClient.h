#pragma once
#include <sstream>

#include "Define.h"
#include "Container/Map.h"
#include "UObject/ObjectMacros.h"
#include "ViewportClient.h"
#include "EngineLoop.h"
#include "EngineBaseTypes.h"

#define MIN_ORTHOZOOM				1.0							/* 2D ortho viewport zoom >= MIN_ORTHOZOOM */
#define MAX_ORTHOZOOM				1e25

extern FEngineLoop GEngineLoop;



struct FViewportCameraTransform
{
public:
    FVector GetForwardVector();
    FVector GetRightVector();
    FVector GetUpVector();

    FViewportCameraTransform();

    /** Sets the transform's location */
    void SetLocation(const FVector& InLocation)
    {
        ViewLocation = InLocation;
    }

    /** Sets the transform's rotation */
    void SetRotation(const FVector& InRotation)
    {
        ViewRotation = InRotation;
    }

    /** Sets the location to look at during orbit */
    void SetLookAt(const FVector& InLookAt)
    {
        LookAt = InLookAt;
    }

    /** Set the ortho zoom amount */
    void SetOrthoZoom(float InOrthoZoom)
    {
        assert(InOrthoZoom >= MIN_ORTHOZOOM && InOrthoZoom <= MAX_ORTHOZOOM);
        OrthoZoom = InOrthoZoom;
    }

    /** Check if transition curve is playing. */
 /*    bool IsPlaying();*/

    /** @return The transform's location */
    FORCEINLINE const FVector& GetLocation() const { return ViewLocation; }

    /** @return The transform's rotation */
    FORCEINLINE const FVector& GetRotation() const { return ViewRotation; }

    /** @return The look at point for orbiting */
    FORCEINLINE const FVector& GetLookAt() const { return LookAt; }

    /** @return The ortho zoom amount */
    FORCEINLINE float GetOrthoZoom() const { return OrthoZoom; }

    /** Current viewport Position. */
    FVector	ViewLocation;
    /** Current Viewport orientation; valid only for perspective projections. */
    FVector ViewRotation;
    FVector	DesiredLocation;
    /** When orbiting, the point we are looking at */
    FVector LookAt;
    /** Viewport start location when animating to another location */
    FVector StartLocation;
    /** Ortho zoom amount */
    float OrthoZoom;
};

class FEditorViewportClient : public FViewportClient
{
public:
    FEditorViewportClient();
    ~FEditorViewportClient();

    virtual void Draw(FViewport* Viewport) override;
    virtual ULevel* GetLevel() const { return NULL; }

    void Initialize(int32 viewportIndex);
    void Tick(float DeltaTime);
    void Release();

    void Input();
    //void ResizeViewport(const DXGI_SWAP_CHAIN_DESC& SwapChainDesc);
    void ResizeViewport(FRect Top, FRect Bottom, FRect Left, FRect Right);

    bool IsSelected(const POINT& Point);

protected:
    /** Camera speed setting */
    int32 CameraSpeedSetting = 1;
    /** Camera speed scalar */
    float CameraSpeedScalar = 1.0f;
    float GridSize;

public:
    static FVector Pivot;
    static float OrthoSize;

    FViewport* GetViewport() { return Viewport; }
    D3D11_VIEWPORT& GetD3DViewport();

    void UpdatePrevMatrix();

    FViewport* Viewport;
    int32 ViewportIndex;
    //카메라
    /** Viewport camera transform data for perspective viewports */
    FViewportCameraTransform		ViewTransformPerspective;
    FViewportCameraTransform        ViewTransformOrthographic;
    // 카메라 정보
    float ViewFOV = 60.0f;
    /** Viewport's stored horizontal field of view (saved in ini files). */
    float FOVAngle = 60.0f;
    float AspectRatio;
    float nearPlane = 1.0f;
    float farPlane = 2000.0f;
    ELevelViewportType ViewportType;
    uint64 ShowFlag;
    EViewModeIndex ViewMode;

    FMatrix View;
    FMatrix Projection;

    FMatrix PrevView;
    FMatrix PrevProjection;

    //Camera Movement
    void CameraMoveForward(float Value);
    void CameraMoveRight(float Value);
    void CameraMoveUp(float Value);
    void CameraRotateYaw(float Value);
    void CameraRotatePitch(float Value);
    void PivotMoveRight(float Value);
    void PivotMoveUp(float Value);

    FVector GetWorldLocation() const;

    FMatrix& GetPrevViewMatrix() { return PrevView; }
    FMatrix& GetPrevProjectionMatrix() { return PrevProjection; }

    FMatrix& GetViewMatrix() { return  View; }
    FMatrix& GetProjectionMatrix() { return Projection; }
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();

    bool IsOrtho() const;
    bool IsPerspective() const;
    ELevelViewportType GetViewportType() const;
    void SetViewportType(ELevelViewportType InViewportType);
    void UpdateOrthoCameraLoc();
    EViewModeIndex GetViewMode() { return ViewMode; }
    void SetViewMode(EViewModeIndex newMode) { ViewMode = newMode; }
    uint64 GetShowFlag() { return ShowFlag; }
    void SetShowFlag(uint64 newMode) { ShowFlag = newMode; }
    bool GetIsOnRBMouseClick() { return bRightMouseDown; }

    //Flag Test Code
    static void SetOthoSize(float Value);

private: // Input
    POINT lastMousePos;
    bool bRightMouseDown = false;

public:
    void LoadConfig(const TMap<FString, FString>& config);
    void SaveConfig(TMap<FString, FString>& config);

private:
    TMap<FString, FString> ReadIniFile(const FString& filePath);
    void WriteIniFile(const FString& filePath, const TMap<FString, FString>& config);

public:
    PROPERTY(int32, CameraSpeedSetting)
    PROPERTY(float, GridSize)
    float GetCameraSpeedScalar() const { return CameraSpeedScalar; };
    void SetCameraSpeedScalar(float value);

private:
    template <typename T>
    T GetValueFromConfig(const TMap<FString, FString>& config, const FString& key, T defaultValue)
    {
        if (const FString* Value = config.Find(key))
        {
            std::istringstream iss(**Value);
            T value;
            if (iss >> value)
            {
                return value;
            }
        }
        return defaultValue;
    }
};

