#pragma once

#include "Define.h"
#include "EngineLoop.h"
#include "Container/Map.h"
#include "HAL/PlatformType.h"
#include "Serialization/Serializer.h"

class UStaticMesh;
struct FManagerOBJ;

struct FLoaderOBJ
{
    // Obj Parsing (*.obj to FObjInfo)
    static bool ParseOBJ(const FString& ObjFilePath, FObjInfo& OutObjInfo);

    // Material Parsing (*.obj to MaterialInfo)
    static bool ParseMaterial(FObjInfo& OutObjInfo, OBJ::FStaticMeshRenderData& OutFStaticMesh);

    // Convert the Raw data to Cooked data (FStaticMeshRenderData)
    static bool ConvertToStaticMesh(const FObjInfo& RawData, OBJ::FStaticMeshRenderData& OutStaticMesh);

    static bool CreateTextureFromFile(const FWString& Filename);

    static void ComputeBoundingBox(const TArray<FStaticMeshVertex>& InVertices, FVector& OutMinVector, FVector& OutMaxVector);
};

struct FManagerOBJ
{
public:
    static OBJ::FStaticMeshRenderData* LoadObjStaticMeshAsset(const FString& PathFileName);

    static void CombineMaterialIndex(OBJ::FStaticMeshRenderData& OutFStaticMesh);

    static bool SaveStaticMeshToBinary(const FWString& FilePath, const OBJ::FStaticMeshRenderData& StaticMesh);

    static bool LoadStaticMeshFromBinary(const FWString& FilePath, OBJ::FStaticMeshRenderData& OutStaticMesh);

    static UMaterial* CreateMaterial(FObjMaterialInfo materialInfo);

    static TMap<FString, UMaterial*>& GetMaterials() { return materialMap; }

    static UMaterial* GetMaterial(FString name);

    static int GetMaterialNum() { return materialMap.Num(); }

    static UStaticMesh* CreateStaticMesh(FString filePath);

    static const TMap<FWString, UStaticMesh*>& GetStaticMeshes() { return staticMeshMap; }

    static UStaticMesh* GetStaticMesh(FWString name);

    static int GetStaticMeshNum() { return staticMeshMap.Num(); }

private:
    inline static TMap<FString, OBJ::FStaticMeshRenderData*> ObjStaticMeshMap;
    inline static TMap<FWString, UStaticMesh*> staticMeshMap;
    inline static TMap<FString, UMaterial*> materialMap;
};