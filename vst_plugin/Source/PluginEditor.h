#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ChatComponent.h"

class FLGPTPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit FLGPTPluginAudioProcessorEditor(FLGPTPluginAudioProcessor&);
    ~FLGPTPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    FLGPTPluginAudioProcessor& audioProcessor;
    ChatComponent chatComponent;

    void showSettings();
    void loadAndApplyApiKey();

    static juce::File   getSettingsFile();
    static void         saveApiKey(const juce::String& key);
    static juce::String loadApiKey();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FLGPTPluginAudioProcessorEditor)
};
