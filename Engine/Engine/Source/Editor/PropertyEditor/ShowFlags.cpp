#include "PropertyEditor/ShowFlags.h"
#include "Level.h"

ShowFlags::ShowFlags()
{
    currentFlags = 31;
}

ShowFlags::~ShowFlags()
{
}

ShowFlags& ShowFlags::GetInstance()
{
	static ShowFlags instance;
	return instance;
}

void ShowFlags::Draw(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
	float controllWindowWidth = static_cast<float>(width) * 0.12f;
	float controllWindowHeight = static_cast<float>(height) * 0.f;

	float controllWindowPosX = (static_cast<float>(width) - controllWindowWidth) * 0.64f;
	float controllWindowPosY = (static_cast<float>(height) - controllWindowHeight) * 0.f;

	// 창 크기와 위치 설정
	ImGui::SetNextWindowPos(ImVec2(controllWindowPosX, controllWindowPosY));
	ImGui::SetNextWindowSize(ImVec2(controllWindowWidth, controllWindowHeight), ImGuiCond_Always);

	if (ImGui::Begin("ShowFlags"))
	{
		const char* items[] = { "AABB", "Primitives", "BillBoardText", "UUID", "Fog", "Gizmo" };
        uint64 ActiveViewportFlags = ActiveViewport->GetShowFlag();
	    bool Selected[IM_ARRAYSIZE(items)] =
        {
	        (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_AABB)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_Primitives)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_BillboardText)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_UUIDText)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_Fog)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_Gizmo)) != 0,
        };  // 각 항목의 체크 상태 저장

		if (ImGui::BeginCombo("Show Flags", "Select Show Flags"))
		{
			for (int i = 0; i < IM_ARRAYSIZE(items); i++)
			{
				ImGui::Checkbox(items[i], &Selected[i]);
			}
			ImGui::EndCombo();
		}
		ActiveViewport->SetShowFlag(ConvertSelectionToFlags(Selected));

	}
	ImGui::End(); // 윈도우 종료
}
uint64 ShowFlags::ConvertSelectionToFlags(const bool selected[])
{
	uint64 flags = static_cast<uint64>(EEngineShowFlags::None);

	if (selected[0])
	{
	    flags |= static_cast<uint64>(EEngineShowFlags::SF_AABB);
	}
	if (selected[1])
	{
	    flags |= static_cast<uint64>(EEngineShowFlags::SF_Primitives);
	}
	if (selected[2])
	{
	    flags |= static_cast<uint64>(EEngineShowFlags::SF_BillboardText);
	}
	if (selected[3])
	{
	    flags |= static_cast<uint64>(EEngineShowFlags::SF_UUIDText);
	}
    if (selected[4])
    {
        flags |= static_cast<uint64>(EEngineShowFlags::SF_Fog);
    }
    if (selected[5])
    {
        flags |= static_cast<uint64>(EEngineShowFlags::SF_Gizmo);
    }
	return flags;
}

void ShowFlags::OnResize(HWND hWnd)
{
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	width = clientRect.right - clientRect.left;
	height = clientRect.bottom - clientRect.top;
}