cmake_minimum_required(VERSION 3.12)

set (CMAKE_CXX_STANDARD 20)
include(${CMAKE_CURRENT_SOURCE_DIR}/pd.build/pd.cmake)

set(HVCC_PATH /opt/homebrew/bin/hvcc)

if(APPLE)
  set(CMAKE_XCODE_BUILD_SYSTEM "12" CACHE STRING "" FORCE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Minimum OS X deployment version")
  if(RUN_CLANG_TIDY)
  set(CMAKE_OSX_ARCHITECTURES "arm64;" CACHE STRING "" FORCE)
  else()
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
  endif()
endif()

project(hvccgen VERSION 0.0.1 LANGUAGES C CXX)

add_subdirectory(GUI)

file(GLOB hvcc_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/hvcc/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hvcc/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/hvcc/*.h
)

set_pd_sources(${CMAKE_CURRENT_SOURCE_DIR}/pure-data/src)
set_pd_external_path(${CMAKE_CURRENT_SOURCE_DIR}/binaries/)

add_pd_external(hvccgen "hvcc~" hvcc.c)
target_sources(hvccgen PUBLIC ${hvcc_sources})
target_link_libraries(hvccgen PUBLIC hvcc_gui)

if(UNIX)
    target_compile_definitions(hvccgen PUBLIC HAVE_LIBDL=1 HVCC_PATH="${HVCC_PATH}")
endif()