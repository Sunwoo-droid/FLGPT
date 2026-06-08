#include "MIDIGenerator.h"

static constexpr int PATTERN_STEPS = 16;

void MIDIGenerator::processBlock(juce::MidiBuffer& midi,
                                  double ppqPosition,
                                  double bpm,
                                  int numSamples,
                                  double sampleRate,
                                  const SessionState& state)
{
    const auto& beat = state.getBeat();
    if (!beat.isLoaded || beat.instruments.isEmpty() || numSamples <= 0) return;

    if (bpm <= 0.0) bpm = (double)beat.tempo;
    if (bpm <= 0.0) bpm = 120.0;

    // Samples per 16th note
    const double samplesPerBeat = sampleRate * 60.0 / bpm;
    const double samplesPerStep = samplesPerBeat / 4.0;
    const int    noteDuration   = juce::jmax(1, (int)(samplesPerStep * 0.45));

    // ppqPosition is in quarter notes; multiply by 4 for 16th-note steps.
    // The pattern loops every PATTERN_STEPS (16) steps = one bar.
    const double globalStepStart = ppqPosition * 4.0;
    const double stepsInBlock    = (double)numSamples / samplesPerStep;

    // Steps whose downbeat falls inside this block (epsilon handles floating-point edge)
    const int firstStep = (int)std::ceil(globalStepStart - 1e-9);
    const int lastStep  = (int)std::ceil(globalStepStart + stepsInBlock - 1e-9);

    for (int step = firstStep; step < lastStep; ++step)
    {
        const int patternStep = ((step % PATTERN_STEPS) + PATTERN_STEPS) % PATTERN_STEPS;

        const double stepOffsetSamples = (step - globalStepStart) * samplesPerStep;
        const int sampleOn  = juce::jlimit(0, numSamples - 1, (int)stepOffsetSamples);
        const int sampleOff = juce::jlimit(sampleOn, numSamples - 1, sampleOn + noteDuration);

        for (int i = 0; i < beat.instruments.size(); ++i)
        {
            const auto& inst = beat.instruments[i];
            if (patternStep >= inst.pattern.size()) continue;
            if (inst.pattern[patternStep] == 0)     continue;
            if (inst.notes.isEmpty())               continue;

            const int channel = isDrum(inst.name) ? 10 : juce::jlimit(1, 9, i + 1);
            const int noteNum = juce::jlimit(0, 127, noteNameToMidi(inst.notes[0]));
            const auto vel    = (uint8_t)juce::jlimit(1, 127, inst.velocity);

            midi.addEvent(juce::MidiMessage::noteOn (channel, noteNum, vel),        sampleOn);
            midi.addEvent(juce::MidiMessage::noteOff(channel, noteNum, (uint8_t)0), sampleOff);
        }
    }
}

bool MIDIGenerator::isDrum(const juce::String& name)
{
    const juce::String lower = name.toLowerCase();
    return lower.contains("kick")  || lower.contains("snare") ||
           lower.contains("hat")   || lower.contains("cymbal") ||
           lower.contains("drum")  || lower.contains("perc")   ||
           lower.contains("clap")  || lower.contains("tom")    ||
           lower.contains("rim")   || lower.contains("crash")  ||
           lower.contains("ride");
}

int MIDIGenerator::noteNameToMidi(const juce::String& noteName)
{
    if (noteName.isEmpty()) return 60;

    static const struct { const char* name; int semitone; } noteTable[] = {
        {"C#", 1}, {"Db", 1}, {"D#", 3}, {"Eb", 3}, {"F#", 6}, {"Gb", 6},
        {"G#", 8}, {"Ab", 8}, {"A#", 10}, {"Bb", 10},
        {"C", 0}, {"D", 2}, {"E", 4}, {"F", 5},
        {"G", 7}, {"A", 9}, {"B", 11}
    };

    const juce::String upper = noteName.toUpperCase();
    int semitone = -1, charsConsumed = 0;

    for (auto& entry : noteTable)
    {
        const juce::String key = juce::String(entry.name).toUpperCase();
        if (upper.startsWith(key) && key.length() > charsConsumed)
        {
            semitone      = entry.semitone;
            charsConsumed = key.length();
        }
    }

    if (semitone < 0 || charsConsumed == 0) return 60;

    const int octave = noteName.substring(charsConsumed).getIntValue();
    return juce::jlimit(0, 127, (octave + 1) * 12 + semitone);
}
