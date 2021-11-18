#pragma once

namespace u4a {

namespace __internal {
// vs
#include "VS_FullscreenTriangle.hlsl.h"
#include "VS_Static.hlsl.h"
#include "VS_Skinned.hlsl.h"
#include "VS_Level.hlsl.h"
#include "VS_Soft.hlsl.h"
#include "VS_Terrain.hlsl.h"
#include "VS_Debug.hlsl.h"
#include "VS_DebugPassthrough.hlsl.h"
// gs
#include "GS_DebugGenNormals.hlsl.h"
// ps
#include "PS_Default.hlsl.h"
#include "PS_Terrain.hlsl.h"
#include "PS_DeferredResolve.hlsl.h"
#include "PS_Debug.hlsl.h"
#include "PS_DebugLit.hlsl.h"
#include "PS_Selection.hlsl.h"
}

// vs
static const void*  sVS_FullscreenTriangleDataPtr   = __internal::sVS_FullscreenTriangleData;
constexpr    size_t sVS_FullscreenTriangleDataLen   = std::size(__internal::sVS_FullscreenTriangleData);
static const void*  sVS_StaticDataPtr               = __internal::sVS_StaticData;
constexpr    size_t sVS_StaticDataLen               = std::size(__internal::sVS_StaticData);
static const void*  sVS_SkinnedDataPtr              = __internal::sVS_SkinnedData;
constexpr    size_t sVS_SkinnedDataLen              = std::size(__internal::sVS_SkinnedData);
static const void*  sVS_LevelDataPtr                = __internal::sVS_LevelData;
constexpr    size_t sVS_LevelDataLen                = std::size(__internal::sVS_LevelData);
static const void*  sVS_SoftDataPtr                 = __internal::sVS_SoftData;
constexpr    size_t sVS_SoftDataLen                 = std::size(__internal::sVS_SoftData);
static const void*  sVS_TerrainDataPtr              = __internal::sVS_TerrainData;
constexpr    size_t sVS_TerrainDataLen              = std::size(__internal::sVS_TerrainData);
static const void*  sVS_DebugDataPtr                = __internal::sVS_DebugData;
constexpr    size_t sVS_DebugDataLen                = std::size(__internal::sVS_DebugData);
static const void*  sVS_DebugPassthroughDataPtr     = __internal::sVS_DebugPassthroughData;
constexpr    size_t sVS_DebugPassthroughDataLen     = std::size(__internal::sVS_DebugPassthroughData);

// gs
static const void*  sGS_DebugGenNormalsDataPtr      = __internal::sGS_DebugGenNormalsData;
constexpr    size_t sGS_DebugGenNormalsDataLen      = std::size(__internal::sGS_DebugGenNormalsData);

// ps
static const void*  sPS_DefaultDataPtr              = __internal::sPS_DefaultData;
constexpr    size_t sPS_DefaultDataLen              = std::size(__internal::sPS_DefaultData);
static const void*  sPS_TerrainDataPtr              = __internal::sPS_TerrainData;
constexpr    size_t sPS_TerrainDataLen              = std::size(__internal::sPS_TerrainData);
static const void*  sPS_DeferredResolveDataPtr      = __internal::sPS_DeferredResolveData;
constexpr    size_t sPS_DeferredResolveDataLen      = std::size(__internal::sPS_DeferredResolveData);
static const void*  sPS_DebugDataPtr                = __internal::sPS_DebugData;
constexpr    size_t sPS_DebugDataLen                = std::size(__internal::sPS_DebugData);
static const void*  sPS_DebugLitDataPtr             = __internal::sPS_DebugLitData;
constexpr    size_t sPS_DebugLitDataLen             = std::size(__internal::sPS_DebugLitData);
static const void*  sPS_SelectionDataPtr            = __internal::sPS_SelectionData;
constexpr    size_t sPS_SelectionDataLen            = std::size(__internal::sPS_SelectionData);

} // namespace u4a
