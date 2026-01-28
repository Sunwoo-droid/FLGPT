cmake_minimum_required(VERSION 3.22)

project(FLGPT VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
add_subdirectory(JUCE)

# Create the plugin target
juce_add_plugin(FLGPT
    COMPANY_NAME "FLGPT"
    PLUGIN_MANUFACTURER_CODE FGPT
    PLUGIN_CODE FGPT
    FORMATS VST3
    PRODUCT_NAME "FLGPT"
    DESCRIPTION "AI-powered beat creation plugin for FL Studio"
    VERSION 1.0.0
)

# Add source files
target_sources(FLGPT PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/SessionState.cpp
    Source/SessionState.h
    Source/MIDIGenerator.cpp
    Source/MIDIGenerator.h
    Source/LLMService.cpp
    Source/LLMService.h
    Source/OpenAIService.cpp
    Source/OpenAIService.h
    Source/AnthropicService.cpp
    Source/AnthropicService.h
    Source/ChatComponent.cpp
    Source/ChatComponent.h
)

# Link JUCE modules
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

# Set plugin properties
set_target_properties(FLGPT PROPERTIES
    JUCE_VST3_CATEGORY Instrument
    JUCE_VST3_SDK_PATH ""
)

# Suppress VST2/VST3 parameter mismatch warning since we only build VST3
target_compile_definitions(FLGPT PRIVATE JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1)

