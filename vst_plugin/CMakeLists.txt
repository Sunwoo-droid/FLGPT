cmake_minimum_required(VERSION 3.22)

project(FLGPT VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(FLGPT
    COMPANY_NAME             "FLGPT"
    PLUGIN_MANUFACTURER_CODE FGPT
    PLUGIN_CODE              FGPT
    FORMATS                  VST3
    PRODUCT_NAME             "FLGPT"
    DESCRIPTION              "AI-powered beat creation plugin for FL Studio"
    VERSION                  1.0.0
    IS_MIDI_EFFECT           TRUE
)

target_sources(FLGPT PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/ChatComponent.cpp
    Source/ChatComponent.h
    Source/SessionState.cpp
    Source/SessionState.h
    Source/MIDIGenerator.cpp
    Source/MIDIGenerator.h
    Source/OpenAIService.cpp
    Source/OpenAIService.h
)

target_compile_definitions(FLGPT PUBLIC JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(FLGPT PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)
