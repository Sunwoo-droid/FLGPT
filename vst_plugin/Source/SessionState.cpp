/*
  ==============================================================================

    SessionState.cpp
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#include "SessionState.h"

//==============================================================================
SessionState::SessionState()
{
}

SessionState::~SessionState()
{
}

//==============================================================================
void SessionState::updateFromBeatData(const juce::var& jsonData)
{
    if (!jsonData.isObject())
        return;
    
    auto* obj = jsonData.getDynamicObject();
    if (obj == nullptr)
        return;
    
    // Parse tempo
    if (obj->hasProperty("tempo"))
        currentBeat.tempo = static_cast<int>(obj->getProperty("tempo"));
    
    // Parse time signature
    if (obj->hasProperty("timeSignature"))
        currentBeat.timeSignature = obj->getProperty("timeSignature").toString();
    
    // Parse key
    if (obj->hasProperty("key"))
        currentBeat.key = obj->getProperty("key").toString();
    
    // Parse style
    if (obj->hasProperty("style"))
        currentBeat.style = obj->getProperty("style").toString();
    
    // Parse description
    if (obj->hasProperty("description"))
        currentBeat.description = obj->getProperty("description").toString();
    
    // Parse instruments
    currentBeat.instruments.clear();
    if (obj->hasProperty("instruments"))
    {
        auto instrumentsVar = obj->getProperty("instruments");
        if (instrumentsVar.isArray())
        {
            auto* instrumentsArray = instrumentsVar.getArray();
            for (auto& instrumentVar : *instrumentsArray)
            {
                currentBeat.instruments.add(parseInstrument(instrumentVar));
            }
        }
    }
    
    notifyBeatDataChanged();
}

//==============================================================================
void SessionState::updateComponent(const juce::String& componentName, const juce::var& componentData)
{
    auto* instrument = findInstrument(componentName);
    
    if (instrument != nullptr)
    {
        // Update existing component
        *instrument = parseInstrument(componentData);
    }
    else
    {
        // Add new component
        addComponent(parseInstrument(componentData));
    }
    
    notifyComponentUpdated(componentName);
}

//==============================================================================
void SessionState::addComponent(const Instrument& instrument)
{
    currentBeat.instruments.add(instrument);
    notifyBeatDataChanged();
}

//==============================================================================
void SessionState::removeComponent(const juce::String& componentName)
{
    for (int i = currentBeat.instruments.size() - 1; i >= 0; --i)
    {
        if (currentBeat.instruments[i].name == componentName)
        {
            currentBeat.instruments.remove(i);
            notifyBeatDataChanged();
            return;
        }
    }
}

//==============================================================================
SessionState::Instrument* SessionState::findInstrument(const juce::String& name)
{
    for (auto& instrument : currentBeat.instruments)
    {
        if (instrument.name == name)
            return &instrument;
    }
    return nullptr;
}

//==============================================================================
const SessionState::Instrument* SessionState::findInstrument(const juce::String& name) const
{
    for (const auto& instrument : currentBeat.instruments)
    {
        if (instrument.name == name)
            return &instrument;
    }
    return nullptr;
}

//==============================================================================
SessionState::Instrument SessionState::parseInstrument(const juce::var& instrumentData)
{
    Instrument instrument;
    
    if (!instrumentData.isObject())
        return instrument;
    
    auto* obj = instrumentData.getDynamicObject();
    if (obj == nullptr)
        return instrument;
    
    // Parse name
    if (obj->hasProperty("name"))
        instrument.name = obj->getProperty("name").toString();
    
    // Parse pattern
    if (obj->hasProperty("pattern"))
        instrument.pattern = parsePattern(obj->getProperty("pattern"));
    
    // Parse notes
    if (obj->hasProperty("notes"))
        instrument.notes = parseNotes(obj->getProperty("notes"));
    
    // Parse velocity
    if (obj->hasProperty("velocity"))
        instrument.velocity = static_cast<int>(obj->getProperty("velocity"));
    
    return instrument;
}

//==============================================================================
juce::Array<int> SessionState::parsePattern(const juce::var& patternData)
{
    juce::Array<int> pattern;
    
    if (patternData.isArray())
    {
        auto* array = patternData.getArray();
        for (auto& value : *array)
        {
            pattern.add(static_cast<int>(value));
        }
    }
    
    return pattern;
}

//==============================================================================
juce::Array<juce::String> SessionState::parseNotes(const juce::var& notesData)
{
    juce::Array<juce::String> notes;
    
    if (notesData.isArray())
    {
        auto* array = notesData.getArray();
        for (auto& value : *array)
        {
            notes.add(value.toString());
        }
    }
    
    return notes;
}

//==============================================================================
void SessionState::saveState(juce::OutputStream& stream)
{
    juce::MemoryOutputStream mos;
    
    // Write tempo
    mos.writeInt(currentBeat.tempo);
    
    // Write time signature
    mos.writeString(currentBeat.timeSignature);
    
    // Write key
    mos.writeString(currentBeat.key);
    
    // Write style
    mos.writeString(currentBeat.style);
    
    // Write description
    mos.writeString(currentBeat.description);
    
    // Write instruments
    mos.writeInt(currentBeat.instruments.size());
    for (const auto& instrument : currentBeat.instruments)
    {
        mos.writeString(instrument.name);
        mos.writeInt(instrument.pattern.size());
        for (auto patternValue : instrument.pattern)
            mos.writeInt(patternValue);
        mos.writeInt(instrument.notes.size());
        for (const auto& note : instrument.notes)
            mos.writeString(note);
        mos.writeInt(instrument.velocity);
    }
    
    stream.write(mos.getData(), mos.getDataSize());
}

//==============================================================================
void SessionState::loadState(juce::InputStream& stream)
{
    // Read tempo
    currentBeat.tempo = stream.readInt();
    
    // Read time signature
    currentBeat.timeSignature = stream.readString();
    
    // Read key
    currentBeat.key = stream.readString();
    
    // Read style
    currentBeat.style = stream.readString();
    
    // Read description
    currentBeat.description = stream.readString();
    
    // Read instruments
    currentBeat.instruments.clear();
    int numInstruments = stream.readInt();
    for (int i = 0; i < numInstruments; ++i)
    {
        Instrument instrument;
        instrument.name = stream.readString();
        
        int patternSize = stream.readInt();
        instrument.pattern.clear();
        for (int j = 0; j < patternSize; ++j)
            instrument.pattern.add(stream.readInt());
        
        int notesSize = stream.readInt();
        instrument.notes.clear();
        for (int j = 0; j < notesSize; ++j)
            instrument.notes.add(stream.readString());
        
        instrument.velocity = stream.readInt();
        
        currentBeat.instruments.add(instrument);
    }
    
    notifyBeatDataChanged();
}

//==============================================================================
void SessionState::addListener(Listener* listener)
{
    listeners.add(listener);
}

//==============================================================================
void SessionState::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

//==============================================================================
void SessionState::notifyBeatDataChanged()
{
    listeners.call([](Listener& l) { l.beatDataChanged(); });
}

//==============================================================================
void SessionState::notifyComponentUpdated(const juce::String& componentName)
{
    listeners.call([componentName](Listener& l) { l.componentUpdated(componentName); });
}

