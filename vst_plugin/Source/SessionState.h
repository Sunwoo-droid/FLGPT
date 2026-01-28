/*
  ==============================================================================

    SessionState.h
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

//==============================================================================
/**
    Manages the current beat session state, including all components
    and their patterns. Enables iterative editing of individual components.
*/
class SessionState
{
public:
    //==============================================================================
    SessionState();
    ~SessionState();
    
    //==============================================================================
    // Beat data structure
    struct Instrument
    {
        juce::String name;
        juce::Array<int> pattern;  // Array of 1s and 0s for 16th note subdivisions
        juce::Array<juce::String> notes;  // Note names like "C2", "D#3"
        int velocity = 100;
    };
    
    struct BeatData
    {
        int tempo = 120;
        juce::String timeSignature = "4/4";
        juce::String key = "C major";
        juce::String style;
        juce::String description;
        juce::Array<Instrument> instruments;
    };
    
    //==============================================================================
    // State management
    void updateFromBeatData(const juce::var& jsonData);
    void updateComponent(const juce::String& componentName, const juce::var& componentData);
    void addComponent(const Instrument& instrument);
    void removeComponent(const juce::String& componentName);
    
    //==============================================================================
    // Getters
    const BeatData& getCurrentBeat() const { return currentBeat; }
    BeatData& getCurrentBeat() { return currentBeat; }
    
    Instrument* findInstrument(const juce::String& name);
    const Instrument* findInstrument(const juce::String& name) const;
    
    //==============================================================================
    // Persistence
    void saveState(juce::OutputStream& stream);
    void loadState(juce::InputStream& stream);
    
    //==============================================================================
    // Listeners
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void beatDataChanged() {}
        virtual void componentUpdated(const juce::String& componentName) {}
    };
    
    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    //==============================================================================
    BeatData currentBeat;
    juce::ListenerList<Listener> listeners;
    
    void notifyBeatDataChanged();
    void notifyComponentUpdated(const juce::String& componentName);
    
    // Helper methods for JSON parsing
    Instrument parseInstrument(const juce::var& instrumentData);
    juce::Array<int> parsePattern(const juce::var& patternData);
    juce::Array<juce::String> parseNotes(const juce::var& notesData);
};

