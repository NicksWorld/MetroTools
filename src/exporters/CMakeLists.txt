cmake_minimum_required(VERSION 3.16)
project(MetroTools LANGUAGES CXX)

add_library(exporters STATIC)
add_library(MetroTools::Exporters ALIAS exporters)
set_target_properties(exporters PROPERTIES CXX_STANDARD 17)
target_include_directories(exporters PUBLIC .)
target_sources(exporters PRIVATE
    ExporterFBX.cpp
    ExporterOBJ.cpp
    ExporterFBX.h
    ExporterOBJ.h)
target_link_libraries(exporters
    PRIVATE
        MetroTools::Common
        MetroTools::Metro
    PUBLIC
        Fbx::Fbx)
