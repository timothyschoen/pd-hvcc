cmake_minimum_required(VERSION 3.2)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(${CMAKE_CURRENT_SOURCE_DIR}/Libraries/pd.build/pd.cmake)

set(HVCC_PATH /opt/homebrew/bin/hvcc)
option(ENABLE_LIBCLANG OFF)

if(APPLE)
  set(CMAKE_XCODE_BUILD_SYSTEM "12" CACHE STRING "" FORCE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")
  set(CMAKE_OSX_ARCHITECTURES "arm64;" CACHE STRING "" FORCE)
endif()

find_package (Python COMPONENTS Interpreter Development)

# handle them in 4 batches because they sometimes get paralellized...
execute_process(
    COMMAND ${CMAKE_COMMAND} -E remove Resources/Resources.zip
    COMMAND ${CMAKE_COMMAND} -E make_directory ./Resources/
    COMMAND ${CMAKE_COMMAND} -E make_directory ./Resources/hvcc
)

execute_process(

    COMMAND ${CMAKE_COMMAND} -E copy /usr/bin/cc ./Resources/cc
    COMMAND ${CMAKE_COMMAND} -E copy ${Python_EXECUTABLE} ./Resources/python
    COMMAND ${CMAKE_COMMAND} -E copy_directory ./Libraries/hvcc ./Resources/hvcc/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar cfv ./Resources/Resources.zip --format=zip -- ./Resources/python ./Resources/cc Resources/hvcc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

project(hvccgen VERSION 0.0.1 LANGUAGES C CXX)

add_subdirectory(Libraries/JUCE)
set(pd_dir ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/pure-data/src)


# HVCC GUI
add_library (hvcc_gui)

# libclang switch
if(ENABLE_LIBCLANG)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (LibClang REQUIRED)


link_directories(${LIBCLANG_LIBDIR})

set(CLANG_LIBS clang clangFrontend clangDriver clangSerialization clangParse
    clangCodeGen  clangSema clangAnalysis clangEdit clangAST clangLex
    clangBasic )

string(REPLACE " " ";" __LIST ${LIBCLANG_CXXFLAGS})
target_compile_options(hvcc_gui PRIVATE ${__LIST})
endif()

target_link_libraries (hvcc_gui PRIVATE juce::juce_core juce::juce_gui_basics juce::juce_graphics ${CLANG_LIBS})

file(GLOB hvcc_gui_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/GUI/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/GUI/*.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/Source/JIT/*.h
    #${CMAKE_CURRENT_SOURCE_DIR}/Source/JIT/*.cpp
)

target_compile_definitions(hvcc_gui PUBLIC JUCE_MODAL_LOOPS_PERMITTED=1)
target_include_directories(hvcc_gui PUBLIC "/Libraries/JUCE/modules" ${pd_dir} ${CMAKE_CURRENT_BINARY_DIR}/juce_binarydata_hvcc_binary_data/JuceLibraryCode/)
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

file(GLOB hvcc_binary_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Resources.zip
    )

juce_add_binary_data(hvcc_binary_data SOURCES ${hvcc_binary_sources})
set_target_properties(hvcc_binary_data PROPERTIES POSITION_INDEPENDENT_CODE ON)

set_pd_sources(${pd_dir})
set_pd_external_path(${CMAKE_CURRENT_SOURCE_DIR}/External/)

add_pd_external(hvccgen "hvcc~" Source/hvcc.c)
target_sources(hvccgen PUBLIC ${hvcc_interface})
target_include_directories(hvccgen PUBLIC ${hvcc_interface_dir})
target_link_libraries(hvccgen PUBLIC hvcc_gui hvcc_binary_data)

if(UNIX)
    target_compile_definitions(hvccgen PUBLIC HAVE_LIBDL=1 HVCC_PATH="${HVCC_PATH}")
endif()