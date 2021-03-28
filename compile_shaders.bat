@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

cd src\engine\shaders

:: Pixel shaders
fxc /E main /T ps_5_0 /O3 /Fh PS_Debug.hlsl.h /Vn sPS_DebugData /Qstrip_reflect /Qstrip_debug PS_Debug.hlsl
fxc /E main /T ps_5_0 /O3 /Fh PS_Default.hlsl.h /Vn sPS_DefaultData /Qstrip_reflect /Qstrip_debug PS_Default.hlsl
fxc /E main /T ps_5_0 /O3 /Fh PS_DeferredResolve.hlsl.h /Vn sPS_DeferredResolveData /Qstrip_reflect /Qstrip_debug PS_DeferredResolve.hlsl
fxc /E main /T ps_5_0 /O3 /Fh PS_Selection.hlsl.h /Vn sPS_SelectionData /Qstrip_reflect /Qstrip_debug PS_Selection.hlsl
fxc /E main /T ps_5_0 /O3 /Fh PS_Terrain.hlsl.h /Vn sPS_TerrainData /Qstrip_reflect /Qstrip_debug PS_Terrain.hlsl

:: Vertex shaders
fxc /E main /T vs_5_0 /O3 /Fh VS_Debug.hlsl.h /Vn sVS_DebugData /Qstrip_reflect /Qstrip_debug VS_Debug.hlsl
fxc /E main /T vs_5_0 /O3 /Fh VS_FullscreenTriangle.hlsl.h /Vn sVS_FullscreenTriangleData /Qstrip_reflect /Qstrip_debug VS_FullscreenTriangle.hlsl
fxc /E main /T vs_5_0 /O3 /Fh VS_Level.hlsl.h /Vn sVS_LevelData /Qstrip_reflect /Qstrip_debug VS_Level.hlsl
fxc /E main /T vs_5_0 /O3 /Fh VS_Skinned.hlsl.h /Vn sVS_SkinnedData /Qstrip_reflect /Qstrip_debug VS_Skinned.hlsl
fxc /E main /T vs_5_0 /O3 /Fh VS_Static.hlsl.h /Vn sVS_StaticData /Qstrip_reflect /Qstrip_debug VS_Static.hlsl
fxc /E main /T vs_5_0 /O3 /Fh VS_Terrain.hlsl.h /Vn sVS_TerrainData /Qstrip_reflect /Qstrip_debug VS_Terrain.hlsl
