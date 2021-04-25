cmake_minimum_required(VERSION 3.16)
project(MetroTools LANGUAGES CXX C)

# external libs
add_subdirectory(libs)

add_subdirectory(src/common)
add_subdirectory(src/metro)
add_subdirectory(src/exporters)
add_subdirectory(src/engine)
add_subdirectory(src/simpleribbon)

add_subdirectory(apps/MetroEX)
add_subdirectory(apps/MetroME)
