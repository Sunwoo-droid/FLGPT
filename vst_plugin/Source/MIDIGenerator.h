#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "SessionState.h"

class MIDIGenerator
{
public:
    // ppqPosition: playhead position in quarter notes (from JUCE AudioPlayHead)
    // bpm: current tempo in beats per minute
    void processBlock(juce::MidiBuffer& midi,
                      double ppqPosition,
                      double bpm,
                      int numSamples,
                      double sampleRate,
                      const SessionState& state);

    static int  noteNameToMidi(const juce::String& noteName);
    static bool isDrum(const juce::String& instrumentName);
};
