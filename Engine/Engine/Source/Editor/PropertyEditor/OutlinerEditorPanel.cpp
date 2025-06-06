﻿#include "OutlinerEditorPanel.h"
#include "EngineLoop.h"
#include "Level.h"
#include "GameFramework/Actor.h"


void OutlinerEditorPanel::Render()
{
    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();

    float PanelWidth = (Width) * 0.2f - 6.0f;
    float PanelHeight = (Height) * 0.3f;

    float PanelPosX = (Width) * 0.8f + 5.0f;
    float PanelPosY = 5.0f;

    ImVec2 MinSize(140, 100);
    ImVec2 MaxSize(FLT_MAX, 500);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    /* Render Start */
    ImGui::Begin("Outliner", nullptr, PanelFlags);

    if (ImGui::TreeNodeEx("Primitives", ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        ULevel* level = GEngineLoop.GetLevel();
        for (AActor* Actor : level->GetActors())
        {
            if (ImGui::Selectable(*Actor->GetActorLabel(), level->GetSelectedTempActor() == Actor))
            {
                level->SetPickedActor(Actor);
                level->SetPickedComponent(nullptr);
                break;
            }
        }
        ImGui::TreePop(); // 트리 닫기
    }
    ImGui::End();
}

void OutlinerEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}
