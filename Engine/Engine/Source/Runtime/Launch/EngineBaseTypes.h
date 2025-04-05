#pragma once

enum EViewModeIndex
{
    VMI_Lit = 0,
    VMI_Unlit = 1,
    VMI_Wireframe = 2,
    VMI_SceneDepth = 3,
};


enum ELevelViewportType
{
    LVT_Perspective = 0,
    /** Top */
    LVT_OrthoXY = 1,
    /** Bottom */
    LVT_OrthoNegativeXY,
    /** Left */
    LVT_OrthoYZ,
    /** Right */
    LVT_OrthoNegativeYZ,
    /** Front */
    LVT_OrthoXZ,
    /** Back */
    LVT_OrthoNegativeXZ,
    LVT_MAX,
    LVT_None = 255,
};
