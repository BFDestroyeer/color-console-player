cmake_minimum_required(VERSION 3.26)
project(color_console_player)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

# Disable deprecated std::shared_ptr atomic API warning because Clang compiler hasn't std::atomic<std::shared_ptr> support
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_definitions(_SILENCE_CXX20_OLD_SHARED_PTR_ATOMIC_SUPPORT_DEPRECATION_WARNING)
endif ()

include_directories("include")

file(GLOB_RECURSE TARGET_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
file(GLOB_RECURSE TARGET_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(color_console_player ${TARGET_HEADERS} ${TARGET_SOURCES})
target_link_libraries(color_console_player ${OpenCV_LIBS})
