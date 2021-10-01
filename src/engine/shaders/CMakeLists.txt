cmake_minimum_required(VERSION 3.16)
project(MetroTools)

set(FXC_BIN ${CMAKE_SOURCE_DIR}/_utils/fxc)
function(CompileHLSLShader source_path shader_type entry_point out includes)
    get_filename_component(source_filename ${source_path} NAME_WE)
    get_filename_component(source_path "${source_path}" ABSOLUTE)
    set(header_path "${CMAKE_CURRENT_BINARY_DIR}/${source_filename}.hlsl.h")
    set(${out} ${header_path} PARENT_SCOPE)
    add_custom_command(OUTPUT ${header_path}
        COMMENT "Compiling shader ${source_path}..."
        COMMAND ${FXC_BIN} /E \"${entry_point}\" /T \"${shader_type}\" /O3 /Fh \"${header_path}\" /Vn s${source_filename}Data /Qstrip_reflect /Qstrip_debug \"${source_path}\"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS "${source_path}" ${${includes}}
    )
endfunction()

set(PIXEL_SHADERS
    PS_Debug.hlsl
    PS_Default.hlsl
    PS_DeferredResolve.hlsl
    PS_Selection.hlsl
    PS_Terrain.hlsl)
set_property(SOURCE ${PIXEL_SHADERS} PROPERTY VS_TOOL_OVERRIDE "None")

set(VERTEX_SHADERS
    VS_Debug.hlsl
    VS_FullscreenTriangle.hlsl
    VS_Level.hlsl
    VS_Skinned.hlsl
    VS_Static.hlsl
    VS_Soft.hlsl
    VS_Terrain.hlsl)
set_property(SOURCE ${VERTEX_SHADERS} PROPERTY VS_TOOL_OVERRIDE "None")

set(SHADER_INCLUDES
    Common.hlsli
    ConstantBuffers.hlsli
    VertexCommon.hlsli)

foreach(ps ${PIXEL_SHADERS})
  CompileHLSLShader(${ps} "ps_5_0" "main" out SHADER_INCLUDES)
  list(APPEND compiled_shaders ${out})
endforeach()

foreach(vs ${VERTEX_SHADERS})
  CompileHLSLShader(${vs} "vs_5_0" "main" out SHADER_INCLUDES)
  list(APPEND compiled_shaders ${out})
endforeach()

add_custom_target(compile_shaders DEPENDS ${compiled_shaders} SOURCES ${PIXEL_SHADERS} ${VERTEX_SHADERS} ${SHADER_INCLUDESS})

add_library(shaders INTERFACE)
add_library(MetroTools::Shaders ALIAS shaders)
target_include_directories(shaders INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(shaders compile_shaders)
