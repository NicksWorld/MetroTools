cmake_minimum_required(VERSION 3.16)
project(MetroTools LANGUAGES CXX C)

# external libs
add_subdirectory(libs)

add_subdirectory(src/common)
add_subdirectory(src/metro)
add_subdirectory(src/exporters)
add_subdirectory(src/engine)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(MetroEX_QT WIN32)
set_target_properties(MetroEX_QT PROPERTIES CXX_STANDARD 17)
target_sources(MetroEX_QT PRIVATE
    res/resources.qrc
    src/main.cpp
    src/mex_settings.cpp
    src/mex_settings.h
    src/hashing.cpp
    src/fileio.cpp
    src/encoding.cpp
    src/ui/imageinfopanel.cpp
    src/ui/imageinfopanel.h
    src/ui/imageinfopanel.ui
    src/ui/imagepanel.cpp
    src/ui/imagepanel.h
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.ui
    src/ui/modelinfopanel.cpp
    src/ui/modelinfopanel.h
    src/ui/modelinfopanel.ui
    src/ui/renderpanel.cpp
    src/ui/renderpanel.h
    src/ui/settingsdlg.cpp
    src/ui/settingsdlg.h
    src/ui/nodes/scriptblocknode.cpp
    src/ui/nodes/scriptblocknode.h
    src/ui/nodes/scriptscene.cpp
    src/ui/nodes/scriptscene.h
    cpp.hint)
target_link_libraries(MetroEX_QT PRIVATE
    MetroTools::Common
    MetroTools::Metro
    MetroTools::Exporters
    MetroTools::Engine

    Qt6::Gui
    Qt6::Widgets
    pugixml
    Xxhash::Xxhash
    NodeEditor::nodes)
