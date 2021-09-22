cmake_minimum_required(VERSION 3.16)
project(MetroTools LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

qt6_add_resources(RESOURCES multitoolbox.qrc)

add_library(MultiToolbox STATIC)
add_library(MetroTools::MultiToolbox ALIAS MultiToolbox)
target_include_directories(MultiToolbox PUBLIC .)
target_sources(MultiToolbox PRIVATE
    multitoolbox.qrc
    multitoolbox.ui
    multitoolbox.cpp
    multitoolbox.h
    titlebutton.cpp
    titlebutton.h
)
target_sources(MultiToolbox PUBLIC ${RESOURCES})
target_link_libraries(MultiToolbox PRIVATE
    Qt6::Gui
    Qt6::Widgets
)
