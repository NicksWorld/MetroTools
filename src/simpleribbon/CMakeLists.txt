cmake_minimum_required(VERSION 3.16)
project(MetroTools LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_library(SimpleRibbon STATIC)
add_library(MetroTools::SimpleRibbon ALIAS SimpleRibbon)
set_target_properties(common PROPERTIES CXX_STANDARD 17)
target_include_directories(SimpleRibbon PUBLIC .)
target_sources(SimpleRibbon PRIVATE
    SimpleRibbon.cpp
    SimpleRibbon.h
    SimpleRibbonButton.cpp
    SimpleRibbonButton.h
    SimpleRibbonGroup.cpp
    SimpleRibbonGroup.h
    SimpleRibbonTab.cpp
    SimpleRibbonTab.h
    SimpleRibbonVBar.cpp
    SimpleRibbonVBar.h
)
target_link_libraries(SimpleRibbon PRIVATE
    Qt6::Gui
    Qt6::Widgets
)
