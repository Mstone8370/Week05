#include "PropertyEditorPanel.h"

#include "Level.h"
#include "Actors/Player.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LightComponent.h"
#include "Components/MotionBlurComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBillboardComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Math/MathUtility.h"
#include "UnrealEd/ImGuiWidget.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"
#include "Components/UFireBallComponent.h"

void PropertyEditorPanel::Render()
{
    /* Pre Setup */
    float PanelWidth = (Width) * 0.2f - 6.0f;
    float PanelHeight = (Height) * 0.65f;

    float PanelPosX = (Width) * 0.8f + 5.0f;
    float PanelPosY = (Height) * 0.3f + 15.0f;

    ImVec2 MinSize(140, 370);
    ImVec2 MaxSize(FLT_MAX, 900);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    /* Render Start */
    ImGui::Begin("Detail", nullptr, PanelFlags);

    AEditorPlayer* EditorPlayer = GEngineLoop.GetLevel()->GetEditorPlayer();

    AActor* TargetActor = GEngineLoop.GetLevel()->GetSelectedTempActor();
    USceneComponent* TargetComponent = GEngineLoop.GetLevel()->GetSelectedTempComponent();


    if (TargetActor == nullptr)
    {
        ImGui::End();
        return;
    }

    if (TargetComponent)
    {
        //UE_LOG(LogLevel::Display, "Name: %s", *TargetComponent->GetName());
        RenderForSceneComponent(TargetActor->GetRootComponent(), TargetComponent, EditorPlayer);

        if (ULightComponentBase* LightComp = Cast<ULightComponentBase>(TargetComponent))
        {
            RenderForLight(LightComp);
        }
        else if (UTextBillboardComponent* TextBillboardComp = Cast<UTextBillboardComponent>(TargetComponent))
        {
            RenderForTextBillboard(TextBillboardComp);
        }
        else if (UTextRenderComponent* TextComp = Cast<UTextRenderComponent>(TargetComponent))
        {
            RenderForTextRender(TextComp);
        }
        else if (UBillboardComponent* BillboardComp = Cast<UBillboardComponent>(TargetComponent))
        {
            RenderForBillboard(BillboardComp);
        }
        else if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(TargetComponent))
        {
            RenderForStaticMesh(StaticMeshComp);
            RenderForMaterial(StaticMeshComp);
        }
        else if (UExponentialHeightFogComponent* ExponentialHeightFogComp = Cast<UExponentialHeightFogComponent>(TargetComponent))
        {
            RenderForExponentialHeightFog(ExponentialHeightFogComp);
        }
        else if (UPointLightComponent* FireBallComponent = Cast<UPointLightComponent>(TargetComponent))
        {
            RenderForFireBall(FireBallComponent);
        }
        else if (UMotionBlurComponent* MotionBlurComponent = Cast<UMotionBlurComponent>(TargetComponent))
        {
            RenderForMotionBlurComponent(MotionBlurComponent);
        }
    }

    ImGui::End();
}

void PropertyEditorPanel::RenderForFireBall(UPointLightComponent* FireBallComp)
{
    if (!FireBallComp)
        return;

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.1f, 0.05f, 1.0f));

    if (ImGui::CollapsingHeader("FireBall Properties", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent(10.0f);

        // 색상 편집기 설정
        static ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha;

        // 현재 색상 값 가져오기 (getter 사용)
        FVector4 currentColor = FireBallComp->GetColor();
        float colorValues[3] = {
            currentColor.x,
            currentColor.y,
            currentColor.z
        };

        // 색상 편집기 표시
        ImGui::Text("Fire Color");
        ImGui::SameLine(150.0f);
        if (ImGui::ColorEdit3("##FireColor", colorValues, colorFlags))
        {
            // 새 색상 값 적용 (setter 사용)
            FVector4 newColor = currentColor;
            newColor.x = colorValues[0];
            newColor.y = colorValues[1];
            newColor.z = colorValues[2];
            FireBallComp->SetColor(newColor);
        }

        // Intensity 슬라이더 (getter/setter 사용)
        float intensity = FireBallComp->GetIntensity();
        ImGui::Text("Intensity");
        ImGui::SameLine(150.0f);
        ImGui::PushItemWidth(200.0f);
        if (ImGui::SliderFloat("##Intensity", &intensity, 0.1f, 100.0f, "%.2f"))
        {
            FireBallComp->SetIntensity(intensity);
        }

        // Radius 슬라이더 (getter/setter 사용)
        float radius = FireBallComp->GetRadius();
        ImGui::Text("Radius");
        ImGui::SameLine(150.0f);
        if (ImGui::SliderFloat("##Radius", &radius, 0.1f, 100.0f, "%.1f"))
        {
            FireBallComp->SetRadius(radius);
        }

        // RadiusFallOff 슬라이더 (getter/setter 사용)
        float falloff = FireBallComp->GetRadiusFallOff();
        ImGui::Text("Falloff");
        ImGui::SameLine(150.0f);
        if (ImGui::SliderFloat("##RadiusFallOff", &falloff, 0.1f, 5.0f, "%.2f"))
        {
            FireBallComp->SetRadiusFallOff(falloff);
        }

        ImGui::PopItemWidth();

        // 미리보기 색상 표시
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 현재 설정에 따른 미리보기 색상 표시
        ImVec4 previewColor = ImVec4(
            currentColor.x * intensity,
            currentColor.y * intensity,
            currentColor.z * intensity,
            1.0f
        );

        ImGui::Unindent(10.0f);
    }

    ImGui::PopStyleColor();
}
void PropertyEditorPanel::RGBToHSV(float r, float g, float b, float& h, float& s, float& v) const
{
    float mx = FMath::Max(r, FMath::Max(g, b));
    float mn = FMath::Min(r, FMath::Min(g, b));
    float delta = mx - mn;

    v = mx;

    if (mx == 0.0f) {
        s = 0.0f;
        h = 0.0f;
        return;
    }
    else {
        s = delta / mx;
    }

    if (delta < 1e-6) {
        h = 0.0f;
    }
    else {
        if (r >= mx) {
            h = (g - b) / delta;
        }
        else if (g >= mx) {
            h = 2.0f + (b - r) / delta;
        }
        else {
            h = 4.0f + (r - g) / delta;
        }
        h *= 60.0f;
        if (h < 0.0f) {
            h += 360.0f;
        }
    }
}

void PropertyEditorPanel::HSVToRGB(float h, float s, float v, float& r, float& g, float& b) const
{
    // h: 0~360, s:0~1, v:0~1
    float c = v * s;
    float hp = h / 60.0f;             // 0~6 구간
    float x = c * (1.0f - fabsf(fmodf(hp, 2.0f) - 1.0f));
    float m = v - c;

    if (hp < 1.0f) { r = c;  g = x;  b = 0.0f; }
    else if (hp < 2.0f) { r = x;  g = c;  b = 0.0f; }
    else if (hp < 3.0f) { r = 0.0f; g = c;  b = x; }
    else if (hp < 4.0f) { r = 0.0f; g = x;  b = c; }
    else if (hp < 5.0f) { r = x;  g = 0.0f; b = c; }
    else { r = c;  g = 0.0f; b = x; }

    r += m;  g += m;  b += m;
}

void PropertyEditorPanel::RenderForSceneComponent(USceneComponent* RootComponent, USceneComponent* SceneComponent, AEditorPlayer* Player)
{
    ImGui::SetItemDefaultFocus();
    // TreeNode 배경색을 변경 (기본 상태)
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

    if (ImGui::TreeNodeEx(*RootComponent->GetOwner()->GetName(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        bool bIsClicked = false;
        RenderForActorHierarchy(RootComponent, bIsClicked);
        ImGui::TreePop(); // 트리 닫기
    }

    if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        Location = SceneComponent->GetLocalLocation();
        Rotation = SceneComponent->GetLocalRotation();
        Scale = SceneComponent->GetLocalScale();

        FImGuiWidget::DrawVec3Control("Location", Location, 0, 85);
        ImGui::Spacing();

        FImGuiWidget::DrawVec3Control("Rotation", Rotation, 0, 85);
        ImGui::Spacing();

        FImGuiWidget::DrawVec3Control("Scale", Scale, 0, 85);
        ImGui::Spacing();

        SceneComponent->SetLocation(Location);
        SceneComponent->SetRotation(Rotation);
        SceneComponent->SetScale(Scale);

        std::string coordiButtonLabel;
        if (Player->GetCoordiMode() == CoordiMode::CDM_WORLD)
            coordiButtonLabel = "World";
        else if (Player->GetCoordiMode() == CoordiMode::CDM_LOCAL)
            coordiButtonLabel = "Local";

        if (ImGui::Button(coordiButtonLabel.c_str(), ImVec2(ImGui::GetWindowContentRegionMax().x * 0.9f, 32)))
        {
            Player->AddCoordiMode();
        }
        ImGui::TreePop(); // 트리 닫기
    }
    ImGui::PopStyleColor();
}

void PropertyEditorPanel::RenderForActorHierarchy(USceneComponent* Component, bool& bClicked)
{
    if (ImGui::TreeNodeEx(*Component->GetName(), ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        if (ImGui::IsItemClicked() && !bClicked) // 클릭 감지
        {
            bClicked = true;
            GEngineLoop.GetLevel()->SetPickedActor(nullptr);
            GEngineLoop.GetLevel()->SetPickedComponent(Component);
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            SelectedComponentForPopup = Component;
            ImGui::OpenPopup("ComponentMenu");
        }

        if (ImGui::BeginPopup("ComponentMenu"))
        {
            if (SelectedComponentForPopup)
            {
                if (ImGui::MenuItem("Add Component"))
                {
                    ImGui::CloseCurrentPopup();
                    bOpenAddComponentModel = true;
                }

                // 루트 컴포넌트가 아닌 경우에만 Delete 메뉴 추가
                AActor* Owner = SelectedComponentForPopup->GetOwner();
                if (Owner && Owner->GetRootComponent() != SelectedComponentForPopup && ImGui::MenuItem("Delete Component"))
                {
                    GEngineLoop.GetLevel()->SetPickedActor(nullptr);
                    GEngineLoop.GetLevel()->SetPickedComponent(nullptr);
                    SelectedComponentForPopup->DestroyComponent();
                }
            }
            ImGui::EndPopup();
        }

        if (bOpenAddComponentModel)
        {
            ImGui::OpenPopup("AddComponentPopup");
            bOpenAddComponentModel = false;
        }
        if (ImGui::BeginPopupModal("AddComponentPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static int CurrentItem = 0;
            TArray<FString> Items = { "StaticMesh Component", "Billboard Component", "Text Component" };

            if (SelectedComponentForPopup)
            {
                // TODO:

                if (ImGui::BeginCombo("##StaticMesh", *Items[CurrentItem], ImGuiComboFlags_None))
                {
                    for (int32 i = 0; i < Items.Num(); ++i)
                    {
                        if (ImGui::Selectable(*Items[i], false))
                        {
                            CurrentItem = i;
                        }
                    }

                    ImGui::EndCombo();
                }
            }

            if (ImGui::Button("Add"))
            {
                if (CurrentItem == 0)
                {
                    auto NewComp = SelectedComponentForPopup->GetOwner()->AddComponent<UStaticMeshComponent>();
                    NewComp->SetupAttachment(SelectedComponentForPopup);
                }
                else if (CurrentItem == 1)
                {
                    auto NewComp = SelectedComponentForPopup->GetOwner()->AddComponent<UBillboardComponent>();
                    NewComp->SetTexture(L"Editor/Icon/S_Actor.png");
                    NewComp->SetupAttachment(SelectedComponentForPopup);
                }
                else if (CurrentItem == 2)
                {
                    auto NewComp = SelectedComponentForPopup->GetOwner()->AddComponent<UTextRenderComponent>();
                    NewComp->SetRowColumnCount(106, 106);
                    NewComp->SetTexture(L"Assets/Texture/font.png");
                    NewComp->SetText(L"Default Text");
                    NewComp->SetupAttachment(SelectedComponentForPopup);
                    NewComp->SetRotation(FVector(90.f, 0.f, 0.f));
                }

                ImGui::CloseCurrentPopup();
            }

            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        TArray<USceneComponent*> Children = Component->GetAttachChildren(); // Copy
        for (USceneComponent* Child : Children)
        {
            if (Child)
            {
                RenderForActorHierarchy(Child, bClicked);
            }
        }
        ImGui::TreePop(); // 트리 닫기
    }
}

void PropertyEditorPanel::RenderForStaticMesh(UStaticMeshComponent* StaticMeshComp)
{
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("Static Mesh", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        ImGui::Text("StaticMesh");
        ImGui::SameLine();

        FString PreviewName;
        if (StaticMeshComp->GetStaticMesh())
        {
            PreviewName = StaticMeshComp->GetStaticMesh()->GetRenderData()->DisplayName;
        }
        else
        {
            PreviewName = TEXT("None");
        }
        const TMap<FWString, UStaticMesh*> Meshes = FManagerOBJ::GetStaticMeshes();
        if (ImGui::BeginCombo("##StaticMesh", GetData(PreviewName), ImGuiComboFlags_None))
        {
            if (ImGui::Selectable(TEXT("None"), false))
            {
                StaticMeshComp->SetStaticMesh(nullptr);
            }

            for (auto Mesh : Meshes)
            {
                if (ImGui::Selectable(GetData(Mesh.Value->GetRenderData()->DisplayName), false))
                {
                    StaticMeshComp->SetStaticMesh(Mesh.Value);
                }
            }

            ImGui::EndCombo();
        }

        ImGui::TreePop();
    }
    ImGui::PopStyleColor();
}


void PropertyEditorPanel::RenderForMaterial(UStaticMeshComponent* StaticMeshComp)
{
    if (StaticMeshComp->GetStaticMesh() == nullptr)
    {
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        for (uint32 i = 0; i < StaticMeshComp->GetNumMaterials(); ++i)
        {
            if (ImGui::Selectable(GetData(StaticMeshComp->GetMaterialSlotNames()[i].ToString()), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    std::cout << GetData(StaticMeshComp->GetMaterialSlotNames()[i].ToString()) << std::endl;
                    SelectedMaterialIndex = i;
                    SelectedStaticMeshComp = StaticMeshComp;
                }
            }
        }

        if (ImGui::Button("    +    ")) {
            bIsCreateMaterial = true;
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("SubMeshes", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        auto subsets = StaticMeshComp->GetStaticMesh()->GetRenderData()->MaterialSubsets;
        for (uint32 i = 0; i < subsets.Num(); ++i)
        {
            std::string temp = "subset " + std::to_string(i);
            if (ImGui::Selectable(temp.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    StaticMeshComp->SetselectedSubMeshIndex(i);
                    SelectedStaticMeshComp = StaticMeshComp;
                }
            }
        }
        std::string temp = "clear subset";
        if (ImGui::Selectable(temp.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
        {
            if (ImGui::IsMouseDoubleClicked(0))
                StaticMeshComp->SetselectedSubMeshIndex(-1);
        }

        ImGui::TreePop();
    }

    ImGui::PopStyleColor();

    if (SelectedMaterialIndex != -1)
    {
        RenderMaterialView(SelectedStaticMeshComp->GetMaterial(SelectedMaterialIndex));
    }
    if (bIsCreateMaterial) {
        RenderCreateMaterialView();
    }
}

void PropertyEditorPanel::RenderMaterialView(UMaterial* Material)
{
    ImGui::SetNextWindowSize(ImVec2(380, 400), ImGuiCond_Once);
    ImGui::Begin("Material Viewer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    static ImGuiSelectableFlags BaseFlag = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoAlpha;

    FVector MatDiffuseColor = Material->GetMaterialInfo().Diffuse;
    FVector MatSpecularColor = Material->GetMaterialInfo().Specular;
    FVector MatAmbientColor = Material->GetMaterialInfo().Ambient;
    FVector MatEmissiveColor = Material->GetMaterialInfo().Emissive;

    float dr = MatDiffuseColor.X;
    float dg = MatDiffuseColor.Y;
    float db = MatDiffuseColor.Z;
    float da = 1.0f;
    float DiffuseColorPick[4] = { dr, dg, db, da };

    ImGui::Text("Material Name |");
    ImGui::SameLine();
    ImGui::Text(*Material->GetMaterialInfo().MaterialName);
    ImGui::Separator();

    ImGui::Text("  Diffuse Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Diffuse##Color", (float*)&DiffuseColorPick, BaseFlag))
    {
        FVector NewColor = { DiffuseColorPick[0], DiffuseColorPick[1], DiffuseColorPick[2] };
        Material->SetDiffuse(NewColor);
    }

    float sr = MatSpecularColor.X;
    float sg = MatSpecularColor.Y;
    float sb = MatSpecularColor.Z;
    float sa = 1.0f;
    float SpecularColorPick[4] = { sr, sg, sb, sa };

    ImGui::Text("Specular Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Specular##Color", (float*)&SpecularColorPick, BaseFlag))
    {
        FVector NewColor = { SpecularColorPick[0], SpecularColorPick[1], SpecularColorPick[2] };
        Material->SetSpecular(NewColor);
    }


    float ar = MatAmbientColor.X;
    float ag = MatAmbientColor.Y;
    float ab = MatAmbientColor.Z;
    float aa = 1.0f;
    float AmbientColorPick[4] = { ar, ag, ab, aa };

    ImGui::Text("Ambient Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Ambient##Color", (float*)&AmbientColorPick, BaseFlag))
    {
        FVector NewColor = { AmbientColorPick[0], AmbientColorPick[1], AmbientColorPick[2] };
        Material->SetAmbient(NewColor);
    }


    float er = MatEmissiveColor.X;
    float eg = MatEmissiveColor.Y;
    float eb = MatEmissiveColor.Z;
    float ea = 1.0f;
    float EmissiveColorPick[4] = { er, eg, eb, ea };

    ImGui::Text("Emissive Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Emissive##Color", (float*)&EmissiveColorPick, BaseFlag))
    {
        FVector NewColor = { EmissiveColorPick[0], EmissiveColorPick[1], EmissiveColorPick[2] };
        Material->SetEmissive(NewColor);
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("Choose Material");
    ImGui::Spacing();

    ImGui::Text("Material Slot Name |");
    ImGui::SameLine();
    ImGui::Text(GetData(SelectedStaticMeshComp->GetMaterialSlotNames()[SelectedMaterialIndex].ToString()));

    ImGui::Text("Override Material |");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    // 메테리얼 이름 목록을 const char* 배열로 변환
    std::vector<const char*> materialChars;
    for (const auto& material : FManagerOBJ::GetMaterials()) {
        materialChars.push_back(*material.Value->GetMaterialInfo().MaterialName);
    }

    //// 드롭다운 표시 (currentMaterialIndex가 범위를 벗어나지 않도록 확인)
    //if (currentMaterialIndex >= FManagerOBJ::GetMaterialNum())
    //    currentMaterialIndex = 0;

    if (ImGui::Combo("##MaterialDropdown", &CurMaterialIndex, materialChars.data(), FManagerOBJ::GetMaterialNum())) {
        UMaterial* material = FManagerOBJ::GetMaterial(materialChars[CurMaterialIndex]);
        SelectedStaticMeshComp->SetMaterial(SelectedMaterialIndex, material);
    }

    if (ImGui::Button("Close"))
    {
        SelectedMaterialIndex = -1;
        SelectedStaticMeshComp = nullptr;
    }

    ImGui::End();
}

void PropertyEditorPanel::RenderForExponentialHeightFog(UExponentialHeightFogComponent* ExponentialHeightFogComponent)
{
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    //if (ImGui::TreeNodeEx("ExponentialHeightFog", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        static ImGuiSelectableFlags BaseFlag = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoAlpha;
        FVector FogColor = ExponentialHeightFogComponent->FogColor;

        float dr = FogColor.X;
        float dg = FogColor.Y;
        float db = FogColor.Z;
        float da = 1.0f;
        float DiffuseColorPick[4] = { dr, dg, db, da };

        //ImGui::Text("Set Property");
        //ImGui::Indent();

        ImGui::Text("  Fog Color");
        ImGui::SameLine();
        if (ImGui::ColorEdit4("Fog##Color", (float*)&DiffuseColorPick, BaseFlag))
        {
            FVector NewColor = { DiffuseColorPick[0], DiffuseColorPick[1], DiffuseColorPick[2] };
            ExponentialHeightFogComponent->FogColor = NewColor;
        }

        ImGui::SliderFloat("FogDensity", &ExponentialHeightFogComponent->FogDensity, 0.001f, 5.0f);
        ImGui::SliderFloat("FogFalloff", &ExponentialHeightFogComponent->FogFalloff, 0.01f, 1.0f);
        ImGui::SliderFloat("FogStartDistance", &ExponentialHeightFogComponent->FogStartDistance, 0.0f, ExponentialHeightFogComponent->FogEndDistance);
        ImGui::SliderFloat("FogEndDistance", &ExponentialHeightFogComponent->FogEndDistance, ExponentialHeightFogComponent->FogStartDistance, 2000.0f);
        ImGui::SliderFloat("DistanceFogIntensity", &ExponentialHeightFogComponent->DistanceFogIntensity, 0.01f, 5.0f);
    }
    ImGui::PopStyleColor();
}

void PropertyEditorPanel::RenderForMotionBlurComponent(UMotionBlurComponent* MotionBlurComponent)
{
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

    if (ImGui::Checkbox("Active", &MotionBlurComponent->bIsActive))
    {
    }
    ImGui::SliderFloat("MaxBlurPixels", &MotionBlurComponent->MaxBlurPixels, 0.f, 64.0f);
    ImGui::SliderFloat("VelocityScale", &MotionBlurComponent->VelocityScale, 0.01f, .5f);
    ImGui::SliderFloat("DepthThreshold", &MotionBlurComponent->DepthThreshold, 0.01f, 0.5f);

    ImGui::PopStyleColor();
}

void PropertyEditorPanel::RenderCreateMaterialView()
{
    ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_Once);
    ImGui::Begin("Create Material Viewer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    static ImGuiSelectableFlags BaseFlag = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoAlpha;

    ImGui::Text("New Name");
    ImGui::SameLine();
    static char materialName[256] = "New Material";
    // 기본 텍스트 입력 필드
    ImGui::SetNextItemWidth(128);
    if (ImGui::InputText("##NewName", materialName, IM_ARRAYSIZE(materialName))) {
        TempMaterialInfo.MaterialName = materialName;
    }

    FVector MatDiffuseColor = TempMaterialInfo.Diffuse;
    FVector MatSpecularColor = TempMaterialInfo.Specular;
    FVector MatAmbientColor = TempMaterialInfo.Ambient;
    FVector MatEmissiveColor = TempMaterialInfo.Emissive;

    float dr = MatDiffuseColor.X;
    float dg = MatDiffuseColor.Y;
    float db = MatDiffuseColor.Z;
    float da = 1.0f;
    float DiffuseColorPick[4] = { dr, dg, db, da };

    ImGui::Text("Set Property");
    ImGui::Indent();

    ImGui::Text("  Diffuse Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Diffuse##Color", (float*)&DiffuseColorPick, BaseFlag))
    {
        FVector NewColor = { DiffuseColorPick[0], DiffuseColorPick[1], DiffuseColorPick[2] };
        TempMaterialInfo.Diffuse = NewColor;
    }

    float sr = MatSpecularColor.X;
    float sg = MatSpecularColor.Y;
    float sb = MatSpecularColor.Z;
    float sa = 1.0f;
    float SpecularColorPick[4] = { sr, sg, sb, sa };

    ImGui::Text("Specular Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Specular##Color", (float*)&SpecularColorPick, BaseFlag))
    {
        FVector NewColor = { SpecularColorPick[0], SpecularColorPick[1], SpecularColorPick[2] };
        TempMaterialInfo.Specular = NewColor;
    }


    float ar = MatAmbientColor.X;
    float ag = MatAmbientColor.Y;
    float ab = MatAmbientColor.Z;
    float aa = 1.0f;
    float AmbientColorPick[4] = { ar, ag, ab, aa };

    ImGui::Text("Ambient Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Ambient##Color", (float*)&AmbientColorPick, BaseFlag))
    {
        FVector NewColor = { AmbientColorPick[0], AmbientColorPick[1], AmbientColorPick[2] };
        TempMaterialInfo.Ambient = NewColor;
    }


    float er = MatEmissiveColor.X;
    float eg = MatEmissiveColor.Y;
    float eb = MatEmissiveColor.Z;
    float ea = 1.0f;
    float EmissiveColorPick[4] = { er, eg, eb, ea };

    ImGui::Text("Emissive Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Emissive##Color", (float*)&EmissiveColorPick, BaseFlag))
    {
        FVector NewColor = { EmissiveColorPick[0], EmissiveColorPick[1], EmissiveColorPick[2] };
        TempMaterialInfo.Emissive = NewColor;
    }
    ImGui::Unindent();

    ImGui::NewLine();
    if (ImGui::Button("Create Material")) {
        FManagerOBJ::CreateMaterial(TempMaterialInfo);
    }

    ImGui::NewLine();
    if (ImGui::Button("Close"))
    {
        bIsCreateMaterial = false;
    }

    ImGui::End();
}

void PropertyEditorPanel::RenderForLight(ULightComponentBase* LightComp)
{
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("SpotLight Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        FVector4 currColor = LightComp->GetColor();

        float r = currColor.x;
        float g = currColor.y;
        float b = currColor.z;
        float a = currColor.a;
        float h, s, v;
        float lightColor[4] = { r, g, b, a };

        // SpotLight Color
        if (ImGui::ColorPicker4("##SpotLight Color", lightColor,
            ImGuiColorEditFlags_DisplayRGB |
            ImGuiColorEditFlags_NoSidePreview |
            ImGuiColorEditFlags_NoInputs |
            ImGuiColorEditFlags_Float))

        {

            r = lightColor[0];
            g = lightColor[1];
            b = lightColor[2];
            a = lightColor[3];
            LightComp->SetColor(FVector4(r, g, b, a));
        }
        RGBToHSV(r, g, b, h, s, v);
        // RGB/HSV
        bool changedRGB = false;
        bool changedHSV = false;

        // RGB
        ImGui::PushItemWidth(50.0f);
        if (ImGui::DragFloat("R##R", &r, 0.001f, 0.f, 1.f)) changedRGB = true;
        ImGui::SameLine();
        if (ImGui::DragFloat("G##G", &g, 0.001f, 0.f, 1.f)) changedRGB = true;
        ImGui::SameLine();
        if (ImGui::DragFloat("B##B", &b, 0.001f, 0.f, 1.f)) changedRGB = true;
        ImGui::Spacing();

        // HSV
        if (ImGui::DragFloat("H##H", &h, 0.1f, 0.f, 360)) changedHSV = true;
        ImGui::SameLine();
        if (ImGui::DragFloat("S##S", &s, 0.001f, 0.f, 1)) changedHSV = true;
        ImGui::SameLine();
        if (ImGui::DragFloat("V##V", &v, 0.001f, 0.f, 1)) changedHSV = true;
        ImGui::PopItemWidth();
        ImGui::Spacing();

        if (changedRGB && !changedHSV)
        {
            // RGB -> HSV
            RGBToHSV(r, g, b, h, s, v);
            LightComp->SetColor(FVector4(r, g, b, a));
        }
        else if (changedHSV && !changedRGB)
        {
            // HSV -> RGB
            HSVToRGB(h, s, v, r, g, b);
            LightComp->SetColor(FVector4(r, g, b, a));
        }

        // Light Radius
        float radiusVal = LightComp->GetRadius();
        if (ImGui::SliderFloat("Radius", &radiusVal, 1.0f, 100.0f))
        {
            LightComp->SetRadius(radiusVal);
        }
        ImGui::TreePop();
    }
    ImGui::PopStyleColor();
}

void PropertyEditorPanel::RenderForTextRender(UTextRenderComponent* TextRenderComp)
{
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("Text Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        if (TextRenderComp) {
            TextRenderComp->SetTexture(L"Assets/Texture/font.png");
            TextRenderComp->SetRowColumnCount(106, 106);
            FWString wText = TextRenderComp->GetText();
            int len = WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string u8Text(len, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, u8Text.data(), len, nullptr, nullptr);

            static char buf[256];
            strcpy_s(buf, u8Text.c_str());

            ImGui::Text("Text: ", buf);
            ImGui::SameLine();
            ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
            if (ImGui::InputText("##Text", buf, 256, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                TextRenderComp->ClearText();
                int wlen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, nullptr, 0);
                FWString newWText(wlen, L'\0');
                if (*(newWText.end()-1) == L'\0')
                {
                    newWText.erase(newWText.end()-1, newWText.end());
                }
                MultiByteToWideChar(CP_UTF8, 0, buf, -1, newWText.data(), wlen);
                TextRenderComp->SetText(newWText);
            }
            ImGui::PopItemFlag();
        }
        ImGui::TreePop();
    }
    ImGui::PopStyleColor();
}

void PropertyEditorPanel::RenderForTextBillboard(UTextBillboardComponent* TextBillboardComp)
{
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("Text Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        if (TextBillboardComp) {
            TextBillboardComp->SetTexture(L"Assets/Texture/font.png");
            TextBillboardComp->SetRowColumnCount(106, 106);
            FWString wText = TextBillboardComp->GetText();
            int len = WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string u8Text(len, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, u8Text.data(), len, nullptr, nullptr);

            static char buf[256];
            strcpy_s(buf, u8Text.c_str());

            ImGui::Text("Text: ", buf);
            ImGui::SameLine();
            ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
            if (ImGui::InputText("##Text", buf, 256, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                TextBillboardComp->ClearText();
                int wlen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, nullptr, 0);
                FWString newWText(wlen, L'\0');
                MultiByteToWideChar(CP_UTF8, 0, buf, -1, newWText.data(), wlen);
                TextBillboardComp->SetText(newWText);
            }
            ImGui::PopItemFlag();
        }
        ImGui::TreePop();
    }
    ImGui::PopStyleColor();
}

void PropertyEditorPanel::RenderForBillboard(UBillboardComponent* BillboardComp)
{
    if (BillboardComp->Texture == nullptr)
    {
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("Sprite", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        ImGui::Text("Sprite");
        ImGui::SameLine();

        const wchar_t* wstr = BillboardComp->Texture->Name.c_str();
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
        char* str = new char[size_needed];
        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, size_needed, nullptr, nullptr);

        FString PreviewName = str;
        const TSet<FString> TextureNames = FEngineLoop::ResourceManager.GetAllTextureKeys();
        if (ImGui::BeginCombo("##Sprite", GetData(PreviewName), ImGuiComboFlags_None))
        {
            for (auto Name : TextureNames)
            {
                if (ImGui::Selectable(*Name, false))
                {
                    BillboardComp->SetTexture(Name.ToWideString());
                }
            }

            ImGui::EndCombo();
        }

        ImGui::TreePop();
    }
    ImGui::PopStyleColor();
}

void PropertyEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}
