/*
  ==============================================================================

    PluginProcessor.h
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include "SessionState.h"
#include "MIDIGenerator.h"
#include "LLMService.h"

//==============================================================================
/**
*/
class FLGPTPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    FLGPTPluginAudioProcessor();
    ~FLGPTPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // FLGPT-specific methods
    SessionState& getSessionState() { return sessionState; }
    MIDIGenerator& getMIDIGenerator() { return midiGenerator; }
    
    void setLLMService(std::unique_ptr<LLMService> service);
    LLMService* getLLMService() { return llmService.get(); }
    
    // Trigger MIDI generation from beat data
    void generateMIDIFromBeat(const juce::var& beatData);
    
    // Update MIDI output when state changes
    void updateMIDIOutput();

private:
    //==============================================================================
    SessionState sessionState;
    MIDIGenerator midiGenerator;
    std::unique_ptr<LLMService> llmService;
    
    // MIDI output buffer for current block
    juce::MidiBuffer midiOutputBuffer;
    
    // Timing for MIDI playback
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    int64_t totalSamplesProcessed = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FLGPTPluginAudioProcessor)
};

