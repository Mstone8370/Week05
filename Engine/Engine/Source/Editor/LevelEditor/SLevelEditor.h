#pragma once
#include "Define.h"
#include "Container/Map.h"

#define ViewportCount 4

class SSplitterH;
class SSplitterV;
class ULevel;
class FEditorViewportClient;

class SLevelEditor
{
public:
    SLevelEditor();
    ~SLevelEditor();
    void Initialize(UINT ScreenWidth, UINT ScreenHeight);
    void Tick(double deltaTime, HWND hWnd);
    void Input(HWND hWnd);
    void Release();

    void SelectViewport(POINT point);
    void SetEnableMultiViewport(bool bIsEnable);
    bool IsMultiViewport() const;

    void ResizeLevel(UINT ScreenWidth, UINT ScreenHeight);

private:
    void ResizeViewports();
private:
    SSplitterH* HSplitter;
    SSplitterV* VSplitter;
    ULevel* Level;
    TArray<std::shared_ptr<FEditorViewportClient>> ViewportClients;
    uint32 ActiveViewportClientIndex;

    bool bLButtonDown = false;
    bool bRButtonDown = false;

    bool bMultiViewportMode;

    POINT lastMousePos;
    float EditorWidth;
    float EditorHeight;

public:
    float GizmoScaleWeight = 1.f;

public:
    TArray<std::shared_ptr<FEditorViewportClient>>& GetViewports() { return ViewportClients; }
    std::shared_ptr<FEditorViewportClient> GetActiveViewportClient() const
    {
        return ViewportClients[ActiveViewportClientIndex];
    }
    void SetViewportClientIndex(uint32 index)
    {
        ActiveViewportClientIndex = index;
    }
    uint32 GetActiveViewportClientIndex() const
    {
        return ActiveViewportClientIndex;
    }

    //Save And Load
private:
    const FString IniFilePath = "editor.ini";
public:
    void LoadConfig();
    void SaveConfig();
private:
    TMap<FString, FString> ReadIniFile(const FString& filePath);
    void WriteIniFile(const FString& filePath, const TMap<FString, FString>& config);

    template <typename T>
    T GetValueFromConfig(const TMap<FString, FString>& config, const FString& key, T defaultValue) {
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

