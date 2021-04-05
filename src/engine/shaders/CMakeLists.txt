cmake_minimum_required(VERSION 3.16)
project(MetroTools)

add_library(shaders INTERFACE)
add_library(MetroTools::Shaders ALIAS shaders)

set(PIXEL_SHADERS
    PS_Debug.hlsl
    PS_Default.hlsl
    PS_DeferredResolve.hlsl
    PS_Selection.hlsl
    PS_Terrain.hlsl)
set_source_files_properties(${PIXEL_SHADERS} PROPERTIES
    VS_SHADER_FLAGS "/O3 /Fh \"${CMAKE_CURRENT_BINARY_DIR}/%(Filename).hlsl.h\" /Qstrip_reflect /Qstrip_debug"
    VS_SHADER_TYPE Pixel
    VS_SHADER_MODEL 5.0
    VS_SHADER_ENTRYPOINT main)
set_property(SOURCE PS_Debug.hlsl PROPERTY VS_SHADER_VARIABLE_NAME sPS_DebugData)
set_property(SOURCE PS_Default.hlsl PROPERTY VS_SHADER_VARIABLE_NAME sPS_DefaultData)
set_property(SOURCE PS_DeferredResolve.hlsl PROPERTY VS_SHADER_VARIABLE_NAME sPS_DeferredResolveData)
set_property(SOURCE PS_Selection.hlsl PROPERTY VS_SHADER_VARIABLE_NAME sPS_SelectionData)
set_property(SOURCE PS_Terrain.hlsl PROPERTY VS_SHADER_VARIABLE_NAME sPS_TerrainData)

set(VERTEX_SHADERS
    VS_Debug.hlsl
    VS_FullscreenTriangle.hlsl
    VS_Level.hlsl
    VS_Skinned.hlsl
    VS_Static.hlsl
    VS_Terrain.hlsl)
set_source_files_properties(${VERTEX_SHADERS} PROPERTIES
    VS_SHADER_FLAGS "/O3 /Fh \"${CMAKE_CURRENT_BINARY_DIR}/%(Filename).hlsl.h\" /Qstrip_reflect /Qstrip_debug"
    VS_SHADER_TYPE Vertex
    VS_SHADER_MODEL 5.0
    VS_SHADER_ENTRYPOINT main)
set_property(SOURCE VS_Debug.hlsl PROPERTY VS_SHADER_VARIABLE_NAME sVS_DebugData)
set_property(SOURCE VS_FullscreenTriangle.hlsl PROPERTY VS_SHADER_VARIABLE_NAME sVS_FullscreenTriangleData)
set_property(SOURCE VS_Level.hlsl PROPERTY VS_SHADER_VARIABLE_NAME sVS_LevelData)
set_property(SOURCE VS_Skinned.hlsl PROPERTY VS_SHADER_VARIABLE_NAME sVS_SkinnedData)
set_property(SOURCE VS_Static.hlsl PROPERTY VS_SHADER_VARIABLE_NAME sVS_StaticData)
set_property(SOURCE VS_Terrain.hlsl PROPERTY VS_SHADER_VARIABLE_NAME sVS_TerrainData)

target_sources(shaders PRIVATE
    ${PIXEL_SHADERS}
    ${VERTEX_SHADERS})
target_include_directories(shaders INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
