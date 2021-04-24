cmake_minimum_required(VERSION 3.16)
project(MetroTools LANGUAGES CXX)

add_library(common STATIC)
add_library(MetroTools::Common ALIAS common)
set_target_properties(common PROPERTIES CXX_STANDARD 17)
target_include_directories(common PUBLIC .)
target_sources(common PRIVATE
    dds_utils.cpp
    dds_utils.h
    log.cpp
    log.h
    mycommon.h
    mymath.h
    mex_settings.cpp
    mex_settings.h)
target_link_libraries(common
    PUBLIC
        Bc7enc16::Bc7enc16
        glm
        pugixml
    PRIVATE
        Stb::Stb)
