cmake_minimum_required(VERSION 3.16)
project(MetroEX LANGUAGES CXX C)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(MetroEX WIN32)
set_target_properties(MetroEX PROPERTIES CXX_STANDARD 17)
target_sources(MetroEX PRIVATE
    res/resources.qrc
    main.cpp
    ui/imageinfopanel.cpp
    ui/imageinfopanel.h
    ui/imageinfopanel.ui
    ui/imagepanel.cpp
    ui/imagepanel.h
    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/mainwindow.ui
    ui/modelinfopanel.cpp
    ui/modelinfopanel.h
    ui/modelinfopanel.ui
    ui/renderpanel.cpp
    ui/renderpanel.h
    ui/settingsdlg.cpp
    ui/settingsdlg.h
    ui/maintoolbar.cpp
    ui/maintoolbar.h
    ui/maintoolbar.ui
    ui/nodes/scriptblocknode.cpp
    ui/nodes/scriptblocknode.h
    ui/nodes/scriptscene.cpp
    ui/nodes/scriptscene.h
    cpp.hint)
target_link_libraries(MetroEX PRIVATE
    MetroTools::Common
    MetroTools::Metro
    MetroTools::Exporters
    MetroTools::Engine

    Qt6::Gui
    Qt6::Widgets
    NodeEditor::nodes)
