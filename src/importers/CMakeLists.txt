cmake_minimum_required(VERSION 3.16)
project(MetroTools LANGUAGES CXX)

add_library(importers STATIC)
add_library(MetroTools::Importers ALIAS importers)
set_target_properties(importers PROPERTIES CXX_STANDARD 17)
target_include_directories(importers PUBLIC .)
target_sources(importers PRIVATE
    ImporterFBX.cpp
    ImporterOBJ.cpp
    ImporterFBX.h
    ImporterOBJ.h)
target_link_libraries(importers
    PRIVATE
        MetroTools::Common
        MetroTools::Metro

        TinyObjLoader::TinyObjLoader
    PUBLIC
        Fbx::Fbx)
