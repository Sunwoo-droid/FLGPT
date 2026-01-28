/*
  ==============================================================================

    PluginEditor.cpp
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FLGPTPluginAudioProcessorEditor::FLGPTPluginAudioProcessorEditor (FLGPTPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set plugin window size
    setSize (800, 600);
    
    // Setup chat component
    chatComponent.setLLMService(audioProcessor.getLLMService());
    chatComponent.setSessionState(&audioProcessor.getSessionState());
    addAndMakeVisible(chatComponent);
}

FLGPTPluginAudioProcessorEditor::~FLGPTPluginAudioProcessorEditor()
{
}

//==============================================================================
void FLGPTPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xff1a1a1a));
    
    // Draw header
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(0, 0, getWidth(), 40);
    
    g.setColour(juce::Colour(0xffffffff));
    g.setFont(juce::Font(juce::FontOptions(20.0f, juce::Font::bold)));
    g.drawText("FLGPT - AI Beat Creator", 10, 5, getWidth() - 20, 30, juce::Justification::left);
}

//==============================================================================
void FLGPTPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Header area
    bounds.removeFromTop(40);
    
    // Chat component fills remaining space
    chatComponent.setBounds(bounds);
}

