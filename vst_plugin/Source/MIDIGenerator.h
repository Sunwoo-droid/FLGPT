/*
  ==============================================================================

    MIDIGenerator.h
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include "SessionState.h"

//==============================================================================
/**
    Converts beat pattern data to MIDI events.
    Handles note mapping, timing, and MIDI channel assignment.
*/
class MIDIGenerator
{
public:
    //==============================================================================
    MIDIGenerator();
    ~MIDIGenerator();
    
    //==============================================================================
    // Generate MIDI events from session state
    void generateMIDIEvents(juce::MidiBuffer& midiBuffer,
                            int64_t startSample,
                            int numSamples,
                            double sampleRate,
                            const SessionState& sessionState);
    
    //==============================================================================
    // Convert note name (e.g., "C2") to MIDI note number (0-127)
    static int noteNameToMIDI(const juce::String& noteName);
    
    //==============================================================================
    // Get MIDI channel for an instrument
    static int getMIDIChannelForInstrument(const juce::String& instrumentName, int instrumentIndex);

private:
    //==============================================================================
    // Note name to MIDI number mapping
    static int getNoteNumber(const juce::String& noteName);
    static int getOctave(const juce::String& noteName);
    
    // Check if instrument is a drum
    static bool isDrumInstrument(const juce::String& instrumentName);
    
    // Generate MIDI events for a single instrument
    void generateInstrumentMIDI(juce::MidiBuffer& midiBuffer,
                                 const SessionState::Instrument& instrument,
                                 int midiChannel,
                                 int64_t startSample,
                                 int numSamples,
                                 double sampleRate,
                                 int tempo);
    
    // Calculate sample position for a pattern step
    int64_t stepToSample(int step, double sampleRate, int tempo) const;
};

