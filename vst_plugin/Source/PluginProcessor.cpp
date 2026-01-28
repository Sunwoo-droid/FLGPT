/*
  ==============================================================================

    PluginProcessor.cpp
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FLGPTPluginAudioProcessor::FLGPTPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // Initialize with default OpenAI service (user can change in settings)
    setLLMService(LLMService::createService("OpenAI"));
}

FLGPTPluginAudioProcessor::~FLGPTPluginAudioProcessor()
{
}

//==============================================================================
const juce::String FLGPTPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FLGPTPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FLGPTPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FLGPTPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FLGPTPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FLGPTPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FLGPTPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FLGPTPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FLGPTPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void FLGPTPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FLGPTPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    totalSamplesProcessed = 0;
}

void FLGPTPluginAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FLGPTPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void FLGPTPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Clear MIDI output buffer
    midiOutputBuffer.clear();
    
    // Generate MIDI events from current session state
    midiGenerator.generateMIDIEvents(midiOutputBuffer, 
                                     totalSamplesProcessed, 
                                     buffer.getNumSamples(), 
                                     currentSampleRate,
                                     sessionState);
    
    // Copy generated MIDI to output
    midiMessages.swapWith(midiOutputBuffer);
    
    // Update sample counter
    totalSamplesProcessed += buffer.getNumSamples();
}

//==============================================================================
bool FLGPTPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* FLGPTPluginAudioProcessor::createEditor()
{
    return new FLGPTPluginAudioProcessorEditor (*this);
}

//==============================================================================
void FLGPTPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save session state
    juce::MemoryOutputStream mos(destData, true);
    sessionState.saveState(mos);
}

void FLGPTPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Load session state
    juce::MemoryInputStream mis(data, static_cast<size_t>(sizeInBytes), false);
    sessionState.loadState(mis);
}

//==============================================================================
void FLGPTPluginAudioProcessor::setLLMService(std::unique_ptr<LLMService> service)
{
    llmService = std::move(service);
}

void FLGPTPluginAudioProcessor::generateMIDIFromBeat(const juce::var& beatData)
{
    // Update session state with new beat data
    sessionState.updateFromBeatData(beatData);
    
    // Trigger MIDI regeneration
    updateMIDIOutput();
}

void FLGPTPluginAudioProcessor::updateMIDIOutput()
{
    // MIDI will be generated in processBlock based on current session state
    // This method can be used to trigger UI updates or other side effects
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FLGPTPluginAudioProcessor();
}

