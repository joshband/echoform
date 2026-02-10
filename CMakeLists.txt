cmake_minimum_required(VERSION 3.15)
project(StereoMemoryDelay VERSION 0.1.0)

# JUCE location - use FetchContent to download if local installation not found
set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../juce")
if(EXISTS "${JUCE_DIR}/CMakeLists.txt")
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    message(STATUS "JUCE not found locally, downloading via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# Enable C++17
set(CMAKE_CXX_STANDARD 17)

# Add plugin
juce_add_plugin(StereoMemoryDelay
    COMPANY_NAME "YourCompany"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Juce
    PLUGIN_CODE SMDL
    FORMATS AU VST3
    PRODUCT_NAME "Stereo Memory Delay"
)

# Define source files
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui_runtime/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui_runtime/*.h"
)

# Add the plugin target
juce_generate_juce_header(StereoMemoryDelay)
target_sources(StereoMemoryDelay PRIVATE ${SOURCE_FILES})

# Link to JUCE modules
target_link_libraries(StereoMemoryDelay PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
)

target_compile_definitions(StereoMemoryDelay PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0
)

option(ENABLE_TESTS "Build tests" ON)
if(ENABLE_TESTS)
    enable_testing()
    juce_add_console_app(MemoryDelayEngineTests
        PRODUCT_NAME "MemoryDelayEngineTests"
    )
    target_sources(MemoryDelayEngineTests PRIVATE
        tests/MemoryDelayEngineTests.cpp
    )
    target_include_directories(MemoryDelayEngineTests PRIVATE src)
    target_link_libraries(MemoryDelayEngineTests PRIVATE
        juce::juce_audio_basics
        juce::juce_core
    )
    juce_generate_juce_header(MemoryDelayEngineTests)
    add_test(NAME MemoryDelayEngineTests COMMAND MemoryDelayEngineTests)
endif()

# QA Harness Integration
option(ENABLE_QA_HARNESS "Build QA harness tests" ON)
if(ENABLE_QA_HARNESS)
    # Add harness as subdirectory
    add_subdirectory(external/qa_harness EXCLUDE_FROM_ALL)

    # Create QA executable
    juce_add_console_app(echoform_qa
        PRODUCT_NAME "EchoformQA"
    )

    target_sources(echoform_qa PRIVATE
        qa/echoform_adapter.cpp
        qa/echoform_adapter.h
        qa/main.cpp
    )

    juce_generate_juce_header(echoform_qa)

    target_include_directories(echoform_qa PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/external/qa_harness/core
    )

    target_link_libraries(echoform_qa PRIVATE
        qa_core
        qa_runners
        qa_scenario_engine
        juce::juce_audio_basics
        juce::juce_core
        juce::juce_dsp
    )

    # Add CTest integration
    add_test(NAME echoform_qa COMMAND echoform_qa)
endif()
