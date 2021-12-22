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
    MetroSessions.h
    MetroSessions.cpp
    ModelAOCalculator.cpp
    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/mainwindow.ui
    ui/mainribbon.cpp
    ui/mainribbon.h
    ui/renderpanel.h
    ui/renderpanel.cpp
    ui/sessionsdlg.cpp
    ui/sessionsdlg.h
    ui/sessionsdlg.ui
    ui/exportmodeldlg.cpp
    ui/exportmodeldlg.h
    ui/exportmodeldlg.ui
    ui/exportfbxdlg.cpp
    ui/exportfbxdlg.h
    ui/exportfbxdlg.ui
    ui/edittpresetsdlg.h
    ui/edittpresetsdlg.cpp
    ui/edittpresetsdlg.ui
    ui/props/materialstringsprop.h
    ui/props/objectpropertybrowser.cpp
    ui/props/objectpropertybrowser.h
    ui/rollouts/boneslistrollout.cpp
    ui/rollouts/boneslistrollout.h
    ui/rollouts/boneslistrollout.ui
    ui/rollouts/motionsrollout.cpp
    ui/rollouts/motionsrollout.h
    ui/rollouts/motionsrollout.ui
    ui/rollouts/facefxrollout.cpp
    ui/rollouts/facefxrollout.h
    ui/rollouts/facefxrollout.ui
    ui/rollouts/paramsrollout.cpp
    ui/rollouts/paramsrollout.h
    ui/rollouts/paramsrollout.ui
    ui/rollouts/modelmeshesrollout.cpp
    ui/rollouts/modelmeshesrollout.h
    ui/rollouts/modelmeshesrollout.ui
    ui/rollouts/modelphysxrollout.cpp
    ui/rollouts/modelphysxrollout.h
    ui/rollouts/modelphysxrollout.ui
    cpp.hint
)
target_link_libraries(MetroME PRIVATE
    MetroTools::Common
    MetroTools::Metro
    MetroTools::Exporters
    MetroTools::Importers
    MetroTools::Engine
    MetroTools::SimpleRibbon
    MetroTools::MultiToolbox

    Qt6::Gui
    Qt6::Widgets

    QtPropertyBrowser::QtPropertyBrowser

    Physx2utils::Physx2utils
    Physx3utils::Physx3utils

    Embree3::Embree3

    d2d1.lib
    dwrite.lib
    embree3.lib
)
