#pragma once
#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"

class USceneComponent;
class AEditorPlayer;
class UTextRenderComponent;
class UTextBillboardComponent;
class UBillboardComponent;
class UStaticMeshComponent;

class PropertyEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    void RGBToHSV(float r, float g, float b, float& h, float& s, float& v) const;
    void HSVToRGB(float h, float s, float v, float& r, float& g, float& b) const;

    void RenderForActor(AActor* PickedActor, AEditorPlayer* Player);
    void RenderForComponents(USceneComponent* Component);
    USceneComponent* SelectedComponentForPopup = nullptr;

    bool bOpenAddComponentModel = false;

    void DisplayClassHierarchy(UClass* InClass);

    /* Static Mesh Settings */
    void RenderForStaticMesh(UStaticMeshComponent* StaticMeshComp);

    /* Materials Settings */
    void RenderForMaterial(UStaticMeshComponent* StaticMeshComp);
    void RenderMaterialView(UMaterial* Material);
    void RenderForExponentialHeightFog(UExponentialHeightFogComponent* ExponentialHeightFogComponent);

    void RenderCreateMaterialView();

    /* Light Settings */
    void RenderForLight(ULightComponentBase* LightComp);

    /* Text Settings */
    void RenderForTextRender(UTextRenderComponent* TextRenderComp);
    void RenderForTextBillboard(UTextBillboardComponent* TextBillboardComp);

    /* Billboard Settings */
    void RenderForBillboard(UBillboardComponent* BillboardComp);

private:
    float Width = 0, Height = 0;
    FVector Location = FVector(0, 0, 0);
    FVector Rotation = FVector(0, 0, 0);
    FVector Scale = FVector(0, 0, 0);

    /* Material Property */
    int SelectedMaterialIndex = -1;
    int CurMaterialIndex = -1;
    UStaticMeshComponent* SelectedStaticMeshComp = nullptr;
    FObjMaterialInfo tempMaterialInfo;
    bool IsCreateMaterial;
};
