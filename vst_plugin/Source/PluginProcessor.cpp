#include "PluginProcessor.h"
#include "PluginEditor.h"

FLGPTPluginAudioProcessor::FLGPTPluginAudioProcessor()
    : AudioProcessor(BusesProperties())  // MIDI effect: no audio buses
{
}

FLGPTPluginAudioProcessor::~FLGPTPluginAudioProcessor() {}

void FLGPTPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = samplesPerBlock;
}

void FLGPTPluginAudioProcessor::releaseResources() {}

bool FLGPTPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    juce::ignoreUnused(layouts);
    return true;
}

void FLGPTPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midi)
{
    juce::ignoreUnused(buffer);
    midi.clear();

    auto* ph = getPlayHead();
    if (!ph) return;

    auto posOpt = ph->getPosition();
    if (!posOpt.hasValue() || !posOpt->getIsPlaying()) return;

    const double ppq = posOpt->getPpqPosition().orFallback(0.0);
    const double bpm = posOpt->getBpm().orFallback(120.0);

    // For MIDI effects the audio buffer has 0 channels but still carries the block size.
    // Fall back to the cached block size if somehow 0 is reported.
    const int numSamples = buffer.getNumSamples() > 0 ? buffer.getNumSamples() : currentBlockSize;
    if (numSamples <= 0) return;

    midiGenerator.processBlock(midi, ppq, bpm, numSamples, currentSampleRate, sessionState);
}

juce::AudioProcessorEditor* FLGPTPluginAudioProcessor::createEditor()
{
    return new FLGPTPluginAudioProcessorEditor(*this);
}

void FLGPTPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    sessionState.saveState(mos);
}

void FLGPTPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::MemoryInputStream mis(data, static_cast<size_t>(sizeInBytes), false);
    sessionState.loadState(mis);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FLGPTPluginAudioProcessor();
}
