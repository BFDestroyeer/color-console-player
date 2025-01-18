cmake_minimum_required(VERSION 3.29)
project(color_console_player)

set(CMAKE_CXX_STANDARD 20)

# OpenCV
find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(color_console_player main.cpp color.hpp)
target_link_libraries( color_console_player ${OpenCV_LIBS} )
