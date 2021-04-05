cmake_minimum_required(VERSION 3.16)
project(MetroTools LANGUAGES CXX)

add_subdirectory(shaders)

add_library(engine STATIC)
add_library(MetroTools::Engine ALIAS engine)
set_target_properties(engine PROPERTIES CXX_STANDARD 17)
target_include_directories(engine PUBLIC ..)
target_sources(engine PRIVATE
    Animator.cpp
    Animator.h
    Camera.cpp
    Camera.h
    LevelGeo.cpp
    LevelGeo.h
    Model.cpp
    Model.h
    Renderer.cpp
    Renderer.h
    RendererTypes.inl
    ResourcesManager.cpp
    ResourcesManager.h
    Scene.cpp
    Scene.h
    Spawner.cpp
    Spawner.h
    Surface.h
    Swapchain.cpp
    Swapchain.h
    Texture.cpp
    Texture.h
    scenenodes/LevelGeoNode.cpp
    scenenodes/LevelGeoNode.h
    scenenodes/ModelNode.cpp
    scenenodes/ModelNode.h
    scenenodes/SceneNode.cpp
    scenenodes/SceneNode.h)
target_link_libraries(engine PRIVATE
    MetroTools::Common
    MetroTools::Metro
    MetroTools::Shaders)
