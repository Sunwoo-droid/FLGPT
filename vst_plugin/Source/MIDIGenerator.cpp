/*
  ==============================================================================

    MIDIGenerator.cpp
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#include "MIDIGenerator.h"

//==============================================================================
MIDIGenerator::MIDIGenerator()
{
}

MIDIGenerator::~MIDIGenerator()
{
}

//==============================================================================
void MIDIGenerator::generateMIDIEvents(juce::MidiBuffer& midiBuffer,
                                        int64_t startSample,
                                        int numSamples,
                                        double sampleRate,
                                        const SessionState& sessionState)
{
    const auto& beat = sessionState.getCurrentBeat();
    
    // Generate MIDI for each instrument
    for (int i = 0; i < beat.instruments.size(); ++i)
    {
        const auto& instrument = beat.instruments[i];
        int midiChannel = getMIDIChannelForInstrument(instrument.name, i);
        
        generateInstrumentMIDI(midiBuffer,
                              instrument,
                              midiChannel,
                              startSample,
                              numSamples,
                              sampleRate,
                              beat.tempo);
    }
}

//==============================================================================
int MIDIGenerator::noteNameToMIDI(const juce::String& noteName)
{
    int noteNumber = getNoteNumber(noteName);
    int octave = getOctave(noteName);
    
    // MIDI note = 12 + (octave * 12) + noteNumber
    int midiNote = 12 + (octave * 12) + noteNumber;
    
    // Clamp to valid MIDI range (0-127)
    return juce::jlimit(0, 127, midiNote);
}

//==============================================================================
int MIDIGenerator::getMIDIChannelForInstrument(const juce::String& instrumentName, int instrumentIndex)
{
    // Drums use channel 10 (MIDI channel 9, 0-indexed)
    if (isDrumInstrument(instrumentName))
        return 9;
    
    // Other instruments use channels 0-8, 11-15 (avoiding channel 10)
    if (instrumentIndex < 9)
        return instrumentIndex;
    else
        return instrumentIndex + 1; // Skip channel 10
}

//==============================================================================
int MIDIGenerator::getNoteNumber(const juce::String& noteName)
{
    juce::String note = noteName.trim();
    
    // Extract note letter (C, D, E, F, G, A, B)
    if (note.length() < 1)
        return 0;
    
    juce::String letter = note.substring(0, 1).toUpperCase();
    
    // Map note letters to semitone offsets
    int baseNote = 0;
    if (letter == "C") baseNote = 0;
    else if (letter == "D") baseNote = 2;
    else if (letter == "E") baseNote = 4;
    else if (letter == "F") baseNote = 5;
    else if (letter == "G") baseNote = 7;
    else if (letter == "A") baseNote = 9;
    else if (letter == "B") baseNote = 11;
    else return 0;
    
    // Check for sharp/flat
    if (note.length() >= 2)
    {
        juce::String secondChar = note.substring(1, 2);
        if (secondChar == "#" || secondChar == "s")
            baseNote += 1;
        else if (secondChar == "b" || secondChar == "f")
            baseNote -= 1;
    }
    
    // Handle enharmonic equivalents
    if (note.contains("Db")) baseNote = 1;
    else if (note.contains("Eb")) baseNote = 3;
    else if (note.contains("Gb")) baseNote = 6;
    else if (note.contains("Ab")) baseNote = 8;
    else if (note.contains("Bb")) baseNote = 10;
    
    return baseNote % 12;
}

//==============================================================================
int MIDIGenerator::getOctave(const juce::String& noteName)
{
    // Extract octave number (should be at the end)
    for (int i = noteName.length() - 1; i >= 0; --i)
    {
        if (juce::CharacterFunctions::isDigit(noteName[i]))
        {
            juce::String octaveStr;
            int j = i;
            while (j >= 0 && juce::CharacterFunctions::isDigit(noteName[j]))
            {
                octaveStr = noteName.substring(j, j + 1) + octaveStr;
                --j;
            }
            return octaveStr.getIntValue();
        }
    }
    
    return 2; // Default to octave 2
}

//==============================================================================
bool MIDIGenerator::isDrumInstrument(const juce::String& instrumentName)
{
    juce::String name = instrumentName.toLowerCase();
    
    return name.contains("kick") ||
           name.contains("snare") ||
           name.contains("hi-hat") ||
           name.contains("hihat") ||
           name.contains("crash") ||
           name.contains("tom") ||
           name.contains("cymbal") ||
           name.contains("drum");
}

//==============================================================================
void MIDIGenerator::generateInstrumentMIDI(juce::MidiBuffer& midiBuffer,
                                            const SessionState::Instrument& instrument,
                                            int midiChannel,
                                            int64_t startSample,
                                            int numSamples,
                                            double sampleRate,
                                            int tempo)
{
    // Calculate samples per 16th note
    // tempo BPM = beats per minute
    // 1 beat = quarter note
    // 1 quarter note = 4 sixteenth notes
    // samples per 16th = (sampleRate * 60) / (tempo * 4)
    double samplesPerSixteenth = (sampleRate * 60.0) / (tempo * 4.0);
    
    // Generate MIDI events for each pattern step
    // We need to handle looping - calculate which pattern cycle we're in
    int patternLength = instrument.pattern.size();
    if (patternLength == 0)
        return;
    
    // Calculate how many complete pattern cycles fit in the buffer range
    double patternLengthInSamples = patternLength * samplesPerSixteenth;
    
    // Find the first pattern cycle that overlaps with our buffer
    int64_t firstCycleStart = (startSample / static_cast<int64_t>(patternLengthInSamples)) * static_cast<int64_t>(patternLengthInSamples);
    
    // Generate events for all overlapping cycles
    for (int64_t cycleStart = firstCycleStart; cycleStart < (startSample + numSamples); cycleStart += static_cast<int64_t>(patternLengthInSamples))
    {
        for (int step = 0; step < patternLength; ++step)
        {
            if (instrument.pattern[step] == 1) // Hit at this step
            {
                // Calculate sample position for this step in this cycle
                int64_t stepSample = cycleStart + static_cast<int64_t>(step * samplesPerSixteenth);
                
                // Check if this step falls within the current buffer range
                if (stepSample >= startSample && stepSample < (startSample + numSamples))
                {
                    // Get note number
                    int noteIndex = step % instrument.notes.size();
                    if (noteIndex < 0 || noteIndex >= instrument.notes.size())
                        noteIndex = 0;
                    juce::String noteName = instrument.notes[noteIndex];
                    int midiNote = noteNameToMIDI(noteName);
                    
                    // Calculate position relative to buffer start
                    int bufferPosition = static_cast<int>(stepSample - startSample);
                    bufferPosition = juce::jlimit(0, numSamples - 1, bufferPosition);
                    
                    // Add note on event
                    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(midiChannel + 1, midiNote, 
                                                                          static_cast<juce::uint8>(juce::jlimit(1, 127, instrument.velocity)));
                    midiBuffer.addEvent(noteOn, bufferPosition);
                    
                    // Add note off event (after 1/16th note duration)
                    int noteOffPosition = static_cast<int>(bufferPosition + samplesPerSixteenth);
                    noteOffPosition = juce::jlimit(0, numSamples - 1, noteOffPosition);
                    if (noteOffPosition > bufferPosition)
                    {
                        juce::MidiMessage noteOff = juce::MidiMessage::noteOff(midiChannel + 1, midiNote);
                        midiBuffer.addEvent(noteOff, noteOffPosition);
                    }
                }
            }
        }
    }
}

//==============================================================================
int64_t MIDIGenerator::stepToSample(int step, double sampleRate, int tempo) const
{
    double samplesPerSixteenth = (sampleRate * 60.0) / (tempo * 4.0);
    return static_cast<int64_t>(step * samplesPerSixteenth);
}

