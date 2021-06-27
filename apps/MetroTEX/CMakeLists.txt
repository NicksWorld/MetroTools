cmake_minimum_required(VERSION 3.16)
project(MetroTEX LANGUAGES CXX C)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/res.rc")
add_executable(MetroTEX WIN32 ${APP_ICON_RESOURCE_WINDOWS})
set_target_properties(MetroTEX PROPERTIES CXX_STANDARD 17)
target_sources(MetroTEX PRIVATE
    res/resources.qrc
    main.cpp
    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/mainwindow.ui
    ui/addtexturewnd.cpp
    ui/addtexturewnd.h
    ui/addtexturewnd.ui
    ui/objectpropertybrowser.cpp
    ui/objectpropertybrowser.h
    ui/imagepanel.cpp
    ui/imagepanel.h
    ui/MetroTextureInfoData.h
    cpp.hint
)
target_link_libraries(MetroTEX PRIVATE
    MetroTools::Common
    MetroTools::Metro

    Qt6::Gui
    Qt6::Widgets

    QtPropertyBrowser::QtPropertyBrowser
)
