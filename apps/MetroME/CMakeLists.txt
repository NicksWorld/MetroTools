cmake_minimum_required(VERSION 3.16)
project(MetroME LANGUAGES CXX C)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(MetroME WIN32)
set_target_properties(MetroME PROPERTIES CXX_STANDARD 17)
target_sources(MetroME PRIVATE
    res/resources.qrc
    main.cpp
    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/mainwindow.ui
    cpp.hint
)
target_link_libraries(MetroME PRIVATE
    MetroTools::Common
    MetroTools::Metro
    MetroTools::Exporters
    MetroTools::Engine
    MetroTools::SimpleRibbon

    Qt6::Gui
    Qt6::Widgets
)
