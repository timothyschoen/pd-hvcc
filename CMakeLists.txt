cmake_minimum_required(VERSION 3.2)

project(hvcc VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(${CMAKE_CURRENT_SOURCE_DIR}/Libraries/pd.build/pd.cmake)

option(ENABLE_LIBCLANG OFF)

if(APPLE)
  set(CMAKE_XCODE_BUILD_SYSTEM "12" CACHE STRING "" FORCE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")
  set(CMAKE_OSX_ARCHITECTURES "arm64;" CACHE STRING "" FORCE)
endif()


# configure build system for external libraries
add_subdirectory(Libraries/JUCE EXCLUDE_FROM_ALL)


set(pd_dir ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/pure-data/src)


# HVCC GUI
add_executable (hvcc_gui)

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


if(ENABLE_LIBCLANG)
file(GLOB hvcc_gui_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/GUI/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/GUI/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/JIT/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/JIT/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/whereami.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/whereami.h
)
else()
file(GLOB hvcc_gui_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/GUI/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/GUI/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/whereami.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/whereami.h
)
endif()

add_compile_definitions(JUCER_ENABLE_GPL_MODE=1 JUCE_DISPLAY_SPLASH_SCREEN=0) 

target_compile_definitions(hvcc_gui PUBLIC JUCE_MODAL_LOOPS_PERMITTED=1 ENABLE_LIBCLANG=${ENABLE_LIBCLANG})
target_include_directories(hvcc_gui PUBLIC "/Libraries/JUCE/modules" ${pd_dir} ${CMAKE_CURRENT_BINARY_DIR}/juce_binarydata_hvcc_binary_data/JuceLibraryCode/)
target_sources(hvcc_gui PRIVATE ${hvcc_gui_sources})


# HVCC External
set(hvcc_interface_dir ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/hvcc_interface)
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

file(GLOB extra_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Interface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/whereami.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/whereami.h
)

file(GLOB hvcc_binary_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/DejaVuSansMono.ttf
    )

juce_add_binary_data(hvcc_binary_data SOURCES ${hvcc_binary_sources})
set_target_properties(hvcc_binary_data PROPERTIES POSITION_INDEPENDENT_CODE ON)

set_pd_sources(${pd_dir})
set_pd_external_path(${CMAKE_CURRENT_SOURCE_DIR}/External/)

add_pd_external(hvcc "hvcc~" Source/hvcc.c)
target_sources(hvcc PUBLIC ${hvcc_interface} ${extra_sources})
target_include_directories(hvcc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/JUCE/modules ${hvcc_interface_dir})
target_link_libraries(hvcc PUBLIC juce::juce_core juce::juce_events juce::juce_data_structures) 

target_link_libraries(hvcc_gui PUBLIC hvcc_binary_data juce::juce_core juce::juce_gui_basics juce::juce_graphics ${CLANG_LIBS})

if(UNIX AND NOT APPLE)
target_link_libraries (hvcc_gui PRIVATE curl)
target_link_libraries (hvcc PRIVATE curl)
endif()

add_dependencies(hvcc_gui hvcc_binary_data)
add_dependencies(hvcc hvcc_gui)


set(PD_LIB_DIR "~/Documents/Pd/externals/hvcc~")


# make a setup in the hvcc build dir for debugging
add_custom_command(TARGET hvcc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hvcc_gui> $<TARGET_FILE_DIR:hvcc>/$<TARGET_FILE_NAME:hvcc_gui>
)

add_custom_command(TARGET hvcc POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Resources/run_hvcc.py $<TARGET_FILE_DIR:hvcc>/run_hvcc.py
)

set(INSTALL_FILES 
    $<TARGET_FILE:hvcc>
    $<TARGET_FILE:hvcc_gui>
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/run_hvcc.py
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Resources/run_hvcc.py $<TARGET_FILE:hvcc_gui> $<TARGET_FILE:hvcc> DESTINATION ${PD_LIB_DIR})


if(UNIX)
    target_compile_definitions(hvcc PUBLIC HAVE_LIBDL=1 HVCC_PATH="${HVCC_PATH}")
endif()