#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

class SessionState
{
public:
    struct Instrument
    {
        juce::String name;
        juce::Array<int> pattern;  // 16 elements: 1 = hit, 0 = rest
        juce::Array<juce::String> notes;
        int velocity = 100;
    };

    struct Beat
    {
        int tempo = 120;
        juce::String key;
        juce::String style;
        juce::Array<Instrument> instruments;
        bool isLoaded = false;
    };

    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void beatChanged() {}
    };

    void updateBeat(const Beat& newBeat);
    void updateComponent(const juce::String& name, const Instrument& updated);

    const Beat& getBeat() const { return beat; }

    void addListener(Listener* l);
    void removeListener(Listener* l);

    void saveState(juce::OutputStream& out);
    void loadState(juce::InputStream& in);

private:
    Beat beat;
    juce::ListenerList<Listener> listeners;
};
