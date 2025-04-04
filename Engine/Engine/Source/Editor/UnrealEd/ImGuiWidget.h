#pragma once
#include <string>
#include <Math/Vector.h>
#include "ImGUI/imgui_internal.h"

struct FImGuiWidget
{
    static void DrawVec3Control(const std::string& label, FVector& values, float resetValue = 0.0f, float columnWidth = 100.0f);
};
