#pragma once
#include "SLevelEditor.h"
#include "SlateCore/Widgets/SWindow.h"
#include "Slate/Widgets/Layout/SSplitter.h"
#include "UnrealClient.h"
#include "UnrealEd/EditorViewportClient.h"
#include "fstream"
#include "sstream"
#include "ostream"

SLevelEditor::SLevelEditor() : HSplitter(nullptr), VSplitter(nullptr),
Level(nullptr), bMultiViewportMode(false)
{
}

SLevelEditor::~SLevelEditor()
{
}

void SLevelEditor::Initialize(UINT ScreenWidth, UINT ScreenHeight)
{
    EditorWidth = ScreenWidth;
    EditorHeight = ScreenHeight;

    for (size_t i = 0; i < ViewportCount; i++)
    {
        std::shared_ptr<FEditorViewportClient> ViewportClient = std::make_shared<FEditorViewportClient>();
        ViewportClients.Add(ViewportClient);
        ViewportClient->Initialize(i);
    }
    ActiveViewportClientIndex = 0;
    VSplitter = new SSplitterV();
    VSplitter->Initialize(FRect(0.0f, EditorHeight * 0.5f - 10, EditorHeight, 20));
    VSplitter->OnDrag(FPoint(0, 0));
    HSplitter = new SSplitterH();
    HSplitter->Initialize(FRect(EditorWidth * 0.5f - 10, 0.0f, 20, EditorWidth));
    HSplitter->OnDrag(FPoint(0, 0));
    LoadConfig();

    SetEnableMultiViewport(bMultiViewportMode);
    ResizeLevel(EditorWidth, EditorHeight);
}

void SLevelEditor::Tick(double deltaTime, HWND hWnd)
{
    if (bMultiViewportMode)
    {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hWnd, &pt);
        if (VSplitter->IsHover(FPoint(pt.x, pt.y)) || HSplitter->IsHover(FPoint(pt.x, pt.y)))
        {
            SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        }
        else
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
        Input(hWnd);
    }

    auto ActiveViewportClient = GetActiveViewportClient();
    ActiveViewportClient->Tick(deltaTime);
}

void SLevelEditor::Input(HWND hWnd)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        if (bLButtonDown == false)
        {
            bLButtonDown = true;
            POINT pt;
            GetCursorPos(&pt);
            GetCursorPos(&lastMousePos);
            ScreenToClient(hWnd, &pt);

            SelectViewport(pt);

            VSplitter->OnPressed(FPoint(pt.x, pt.y));
            HSplitter->OnPressed(FPoint(pt.x, pt.y));
        }
        else
        {
            POINT currentMousePos;
            GetCursorPos(&currentMousePos);

            // 마우스 이동 차이 계산
            int32 deltaX = currentMousePos.x - lastMousePos.x;
            int32 deltaY = currentMousePos.y - lastMousePos.y;

            if (VSplitter->IsPressing())
            {
                VSplitter->OnDrag(FPoint(deltaX, deltaY));
            }
            if (HSplitter->IsPressing())
            {
                HSplitter->OnDrag(FPoint(deltaX, deltaY));
            }

            if (VSplitter->IsPressing() || HSplitter->IsPressing())
            {
                ResizeViewports();
                FEngineLoop::GraphicDevice.OnResize(hWnd, GEngineLoop.GetLevelEditor());
            }
            lastMousePos = currentMousePos;
        }
    }
    else
    {
        bLButtonDown = false;
        VSplitter->OnReleased();
        HSplitter->OnReleased();
    }
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
    {
        if (!bRButtonDown)
        {
            bRButtonDown = true;
            POINT pt;
            GetCursorPos(&pt);
            GetCursorPos(&lastMousePos);
            ScreenToClient(hWnd, &pt);

            SelectViewport(pt);
        }
    }
    else
    {
        bRButtonDown = false;
    }
}

void SLevelEditor::Release()
{
    SaveConfig();
    delete VSplitter;
    delete HSplitter;
}

void SLevelEditor::SelectViewport(POINT point)
{
    for (uint32 i = 0; i < ViewportClients.Num(); i++)
    {
        if (ViewportClients[i]->IsSelected(point))
        {
            SetViewportClientIndex(i);
            break;
        }
    }
}

void SLevelEditor::ResizeLevel(UINT ScreenWidth, UINT ScreenHeight)
{
    float PrevWidth = EditorWidth;
    float PrevHeight = EditorHeight;

    EditorWidth = ScreenWidth;
    EditorHeight = ScreenHeight;

    //HSplitter 에는 바뀐 width 비율이 들어감
    HSplitter->OnResize(EditorWidth/PrevWidth, EditorHeight);
    //HSplitter 에는 바뀐 Height 비율이 들어감
    VSplitter->OnResize(EditorWidth, EditorHeight/PrevHeight);
    ResizeViewports();
}

void SLevelEditor::ResizeViewports()
{
    if (bMultiViewportMode)
    {
        for (const auto& ViewportClient : ViewportClients)
        {
            ViewportClient->ResizeViewport(VSplitter->SideLT->Rect, VSplitter->SideRB->Rect,
                HSplitter->SideLT->Rect, HSplitter->SideRB->Rect);
        }
    }
    else
    {
        GetActiveViewportClient()->GetViewport()->ResizeViewport(FRect(0.0f, 0.0f, EditorWidth, EditorHeight));
    }
}

void SLevelEditor::SetEnableMultiViewport(bool bIsEnable)
{
    bMultiViewportMode = bIsEnable;

    ResizeViewports();
}

bool SLevelEditor::IsMultiViewport() const
{
    return bMultiViewportMode;
}

void SLevelEditor::LoadConfig()
{
    auto config = ReadIniFile(IniFilePath);
    auto ActiveViewportClient = GetActiveViewportClient();
    ActiveViewportClient->Pivot.X = GetValueFromConfig(config, "OrthoPivotX", 0.0f);
    ActiveViewportClient->Pivot.Y = GetValueFromConfig(config, "OrthoPivotY", 0.0f);
    ActiveViewportClient->Pivot.Z = GetValueFromConfig(config, "OrthoPivotZ", 0.0f);
    ActiveViewportClient->OrthoSize = GetValueFromConfig(config, "OrthoZoomSize", 10.0f);

    SetViewportClientIndex(GetValueFromConfig(config, "ActiveViewportIndex", 0));
    bMultiViewportMode = GetValueFromConfig(config, "bMutiView", false);
    for (const auto& ViewportClient : ViewportClients)
    {
        ViewportClient->LoadConfig(config);
    }
    if (HSplitter)
        HSplitter->LoadConfig(config);
    if (VSplitter)
        VSplitter->LoadConfig(config);

}

void SLevelEditor::SaveConfig()
{
    TMap<FString, FString> config;
    if (HSplitter)
        HSplitter->SaveConfig(config);
    if (VSplitter)
        VSplitter->SaveConfig(config);
    for (const auto& ViewportClient : ViewportClients)
    {
        ViewportClient->SaveConfig(config);
    }
    auto ActiveViewportClient = GetActiveViewportClient();
    ActiveViewportClient->SaveConfig(config);
    config["bMutiView"] = std::to_string(bMultiViewportMode);
    config["ActiveViewportIndex"] = std::to_string(ActiveViewportClient->ViewportIndex);
    config["ScreenWidth"] = std::to_string(ActiveViewportClient->ViewportIndex);
    config["ScreenHeight"] = std::to_string(ActiveViewportClient->ViewportIndex);
    config["OrthoPivotX"] = std::to_string(ActiveViewportClient->Pivot.X);
    config["OrthoPivotY"] = std::to_string(ActiveViewportClient->Pivot.Y);
    config["OrthoPivotZ"] = std::to_string(ActiveViewportClient->Pivot.Z);
    config["OrthoZoomSize"] = std::to_string(ActiveViewportClient->OrthoSize);
    WriteIniFile(IniFilePath, config);
}

TMap<FString, FString> SLevelEditor::ReadIniFile(const FString& filePath)
{
    TMap<FString, FString> config;
    std::ifstream file(*filePath);
    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '[' || line[0] == ';') continue;
        std::istringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value))
        {
            config[key] = value;
        }
    }
    return config;
}

void SLevelEditor::WriteIniFile(const FString& filePath, const TMap<FString, FString>& config)
{
    std::ofstream file(*filePath);
    for (const auto& pair : config)
    {
        file << *pair.Key << "=" << *pair.Value << "\n";
    }
}

