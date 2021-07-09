cmake_minimum_required(VERSION 3.16)
project(MetroPAK LANGUAGES CXX C)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/res.rc")
add_executable(MetroPAK WIN32 ${APP_ICON_RESOURCE_WINDOWS})
set_target_properties(MetroPAK PROPERTIES CXX_STANDARD 17)
target_sources(MetroPAK PRIVATE
    res/resources.qrc
    main.cpp
    metropackunpack.cpp
    metropackunpack.h
    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/mainwindow.ui
    cpp.hint
)
target_link_libraries(MetroPAK PRIVATE
    MetroTools::Common
    MetroTools::Metro

    Qt6::Gui
    Qt6::Widgets
)
