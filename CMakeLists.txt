cmake_minimum_required(VERSION 3.15)
project(StereoMemoryDelay VERSION 0.1.0)

# JUCE location (modify this path to point to your JUCE installation)
set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../juce")
add_subdirectory(${JUCE_DIR} JUCE)

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
