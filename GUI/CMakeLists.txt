

add_subdirectory(JUCE)

add_library (hvcc_gui)
target_link_libraries (hvcc_gui PRIVATE juce::juce_core juce::juce_gui_basics juce::juce_graphics) 

file(GLOB hvcc_gui_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

target_compile_definitions(hvcc_gui PUBLIC JUCE_MODAL_LOOPS_PERMITTED=1)
target_include_directories(hvcc_gui PUBLIC "/JUCE/modules")
target_sources(hvcc_gui PRIVATE ${hvcc_gui_sources})