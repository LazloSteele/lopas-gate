cmake_minimum_required(VERSION 3.22)
project(LopasGate VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(LopasGate
    COMPANY_NAME            "LopasGate"
    PLUGIN_MANUFACTURER_CODE Lpgt
    PLUGIN_CODE             Blpg
    FORMATS                 VST3 Standalone
    PRODUCT_NAME            "Lopas Gate"
    IS_MIDI_EFFECT          FALSE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_SYNTH                FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES         "Fx" "Filter"
)

juce_generate_juce_header(LopasGate)

target_sources(LopasGate PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
)

target_include_directories(LopasGate PRIVATE src)

target_compile_definitions(LopasGate PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(LopasGate PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
