/*
  ==============================================================================

    PluginEditor.h
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "ChatComponent.h"

//==============================================================================
/**
*/
class FLGPTPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FLGPTPluginAudioProcessorEditor (FLGPTPluginAudioProcessor&);
    ~FLGPTPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    FLGPTPluginAudioProcessor& audioProcessor;
    ChatComponent chatComponent;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FLGPTPluginAudioProcessorEditor)
};

