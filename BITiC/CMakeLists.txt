
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -w -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")

if (VERBOSE_CMAKE)
  message("-- Compiler flags:")
  message("   C++ Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
  message("   C++ compilation flags: ${CMAKE_CXX_FLAGS}")

  message("-- Build type: ${CMAKE_BUILD_TYPE}")
  if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message("   C++ debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
  endif()
  if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message("   C++ release flags: ${CMAKE_CXX_FLAGS_RELEASE}")
  endif()
endif()

add_subdirectory(src)
if (WIN32)
  add_library(BITiC STATIC ${BITiC_SRC})
else ()
  add_library(BITiC SHARED ${BITiC_SRC})
endif()

target_include_directories(BITiC
  PRIVATE src
  PUBLIC include
)
