#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "SessionState.h"
#include "MIDIGenerator.h"
#include "OpenAIService.h"

class FLGPTPluginAudioProcessor : public juce::AudioProcessor
{
public:
    FLGPTPluginAudioProcessor();
    ~FLGPTPluginAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool   hasEditor()           const override { return true; }
    const  juce::String getName() const override { return JucePlugin_Name; }
    bool   acceptsMidi()         const override { return true; }
    bool   producesMidi()        const override { return true; }
    bool   isMidiEffect()        const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms()                          override { return 1; }
    int getCurrentProgram()                       override { return 0; }
    void setCurrentProgram(int)                   override {}
    const juce::String getProgramName(int)        override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    SessionState&   getSessionState()   { return sessionState; }
    OpenAIService&  getOpenAIService()  { return openAIService; }

private:
    SessionState  sessionState;
    MIDIGenerator midiGenerator;
    OpenAIService openAIService;

    double currentSampleRate = 44100.0;
    int    currentBlockSize  = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FLGPTPluginAudioProcessor)
};
