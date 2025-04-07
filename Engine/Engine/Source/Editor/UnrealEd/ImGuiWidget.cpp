#include "ImGuiWidget.h"

void FImGuiWidget::DrawVec3Control(const std::string& label, FVector& values, float resetValue, float columnWidth)
{
    ImGuiIO& IO = ImGui::GetIO();
    auto BoldFont = IO.Fonts->Fonts[0];

    ImGui::PushID(label.c_str());

    // 현재 윈도우 크기 가져오기
    float WindowWidth = ImGui::GetWindowWidth();

    // 컨트롤러 하나의 너비 계산 (대략적인 값)
    float ControlWidth = columnWidth + // 라벨 컬럼
                        3.0f * (5.0f + ImGui::CalcItemWidth() + 5.0f) + // 버튼(5) + DragFloat + 간격(5)
                        GImGui->Style.ItemSpacing.x * 2; // 추가적인 아이템 간격

    // 가운데 정렬을 위한 오프셋 계산
    float Offset = (WindowWidth - ControlWidth) * 0.5f;
    if (Offset > 0)
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + Offset);
    }

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text(label.c_str());
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

    float LineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 ButtonSize = { 5.0f, LineHeight };

    // X 컨트롤
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    ImGui::PushFont(BoldFont);
    if (ImGui::Button("I", ButtonSize))
        values.X = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##X", &values.X, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine(0, 5);

    // Y 컨트롤
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0f, 0.5f, 0.0f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    ImGui::PushFont(BoldFont);
    if (ImGui::Button("II", ButtonSize))
        values.Y = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Y", &values.Y, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine(0, 5);

    // Z 컨트롤
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0f, 0.0f, 1.0f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushFont(BoldFont);
    if (ImGui::Button("III", ButtonSize))
    {
        values.Z = resetValue;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Z", &values.Z, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    ImGui::PopStyleVar(2);
    ImGui::Columns(1);
    ImGui::PopID();
}
