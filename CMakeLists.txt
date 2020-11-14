cmake_minimum_required(VERSION 3.13.4)

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "CMake build type: ${CMAKE_BUILD_TYPE}")

project(BITiC)

set(USER_SOURCE "demo.cpp" CACHE STRING "Source to build with BITiC library")
set(OUTPUT_EXE "demo" CACHE STRING "Executable to compile to")

set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(BITiC_DIR "BITiC" CACHE STRING
    "Name of the directory containing the BITiC library sources. It must be located in the same directory as the root CMakeLists.txt")
add_subdirectory(${BITiC_DIR})
set(BITiC_LIB_PATH "${CMAKE_CURRENT_BINARY_DIR}/${BITiC_DIR}")
set(BITiC_LIB_EXACT "${BITiC_LIB_PATH}/libBITiC.so")

message(STATUS "Compiling ${USER_SOURCE} to executable ${OUTPUT_EXE}")

add_executable(${OUTPUT_EXE} ${USER_SOURCE})

if (WIN32)
  target_link_libraries(${OUTPUT_EXE} BITiC)
else ()
  target_link_libraries(${OUTPUT_EXE} BITiC m)
endif()
