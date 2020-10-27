cmake_minimum_required(VERSION 3.13.4)

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "CMake build type: ${CMAKE_BUILD_TYPE}")

project(CppBmpToolkits)

set(USER_SOURCE "demo.cpp" CACHE STRING "Source to build with CppBmpToolkits library")
set(OUTPUT_EXE "demo" CACHE STRING "Executable to compile to")

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CppBmpToolkits_DIR "CppBmpToolkits" CACHE STRING
    "Name of the directory containing the CppBmpToolkits library sources. It must be located in the same directory as the root CMakeLists.txt")
add_subdirectory(${CppBmpToolkits_DIR})
set(CppBmpToolkits_LIB_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CppBmpToolkits_DIR}")
set(CppBmpToolkits_LIB_EXACT "${CppBmpToolkits_LIB_PATH}/libCppBmpToolkits.so")

message(STATUS "Compiling ${USER_SOURCE} to executable ${OUTPUT_EXE}")

add_executable(${OUTPUT_EXE} ${USER_SOURCE})

if (WIN32)
  target_link_libraries(${OUTPUT_EXE} CppBmpToolkits)
else ()
  target_link_libraries(${OUTPUT_EXE} CppBmpToolkits m)
endif()
