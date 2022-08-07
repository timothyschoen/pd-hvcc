cmake_minimum_required(VERSION 3.12)

set (CMAKE_CXX_STANDARD 20)
include(${CMAKE_CURRENT_SOURCE_DIR}/Libraries/pd.build/pd.cmake)

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

add_subdirectory(Libraries/JUCE)
set(pd_dir ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/pure-data/src)

# HVCC GUI
add_library (hvcc_gui)
target_link_libraries (hvcc_gui PRIVATE juce::juce_core juce::juce_gui_basics juce::juce_graphics) 

file(GLOB hvcc_gui_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/GUI/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/GUI/*.cpp
)

target_compile_definitions(hvcc_gui PUBLIC JUCE_MODAL_LOOPS_PERMITTED=1)
target_include_directories(hvcc_gui PUBLIC "/Libraries/JUCE/modules" ${pd_dir})
target_sources(hvcc_gui PRIVATE ${hvcc_gui_sources})

# HVCC External
set(hvcc_interface_dir ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/hvcc/hvcc/generators/ir2c/static)
file(GLOB hvcc_interface
${hvcc_interface_dir}/HeavyContext.cpp
${hvcc_interface_dir}/HvControlBinop.c
${hvcc_interface_dir}/HvControlCast.c
${hvcc_interface_dir}/HvControlDelay.c
${hvcc_interface_dir}/HvControlIf.c
${hvcc_interface_dir}/HvControlPack.c
${hvcc_interface_dir}/HvControlPrint.c
${hvcc_interface_dir}/HvControlRandom.c
${hvcc_interface_dir}/HvControlSlice.c
${hvcc_interface_dir}/HvControlSystem.c
${hvcc_interface_dir}/HvControlTabhead.c
${hvcc_interface_dir}/HvControlTabread.c
${hvcc_interface_dir}/HvControlTabwrite.c
${hvcc_interface_dir}/HvControlUnop.c
${hvcc_interface_dir}/HvControlVar.c
${hvcc_interface_dir}/HvHeavy.cpp
${hvcc_interface_dir}/HvLightPipe.c
${hvcc_interface_dir}/HvMessage.c
${hvcc_interface_dir}/HvMessagePool.c
${hvcc_interface_dir}/HvMessageQueue.c
${hvcc_interface_dir}/HvSignalBiquad.c
${hvcc_interface_dir}/HvSignalConvolution.c
${hvcc_interface_dir}/HvSignalCPole.c
${hvcc_interface_dir}/HvSignalDel1.c
${hvcc_interface_dir}/HvSignalEnvelope.c
${hvcc_interface_dir}/HvSignalLine.c
${hvcc_interface_dir}/HvSignalLorenz.c
${hvcc_interface_dir}/HvSignalPhasor.c
${hvcc_interface_dir}/HvSignalRPole.c
${hvcc_interface_dir}/HvSignalSamphold.c
${hvcc_interface_dir}/HvSignalSample.c
${hvcc_interface_dir}/HvSignalTabread.c
${hvcc_interface_dir}/HvSignalTabwrite.c
${hvcc_interface_dir}/HvSignalVar.c
${hvcc_interface_dir}/HvTable.c
${hvcc_interface_dir}/HvUtils.c
)


set_pd_sources(${pd_dir})
set_pd_external_path(${CMAKE_CURRENT_SOURCE_DIR}/External/)

add_pd_external(hvccgen "hvcc~" Source/hvcc.c)
target_sources(hvccgen PUBLIC ${hvcc_interface})
target_include_directories(hvccgen PUBLIC ${hvcc_interface_dir})
target_link_libraries(hvccgen PUBLIC hvcc_gui)

if(UNIX)
    target_compile_definitions(hvccgen PUBLIC HAVE_LIBDL=1 HVCC_PATH="${HVCC_PATH}")
endif()