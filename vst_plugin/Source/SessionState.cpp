#include "SessionState.h"

void SessionState::updateBeat(const Beat& newBeat)
{
    beat = newBeat;
    beat.isLoaded = true;
    listeners.call([](Listener& l) { l.beatChanged(); });
}

void SessionState::updateComponent(const juce::String& name, const Instrument& updated)
{
    for (auto& inst : beat.instruments)
    {
        if (inst.name.equalsIgnoreCase(name))
        {
            inst = updated;
            listeners.call([](Listener& l) { l.beatChanged(); });
            return;
        }
    }
    beat.instruments.add(updated);
    listeners.call([](Listener& l) { l.beatChanged(); });
}

void SessionState::addListener(Listener* l)   { listeners.add(l); }
void SessionState::removeListener(Listener* l) { listeners.remove(l); }

void SessionState::saveState(juce::OutputStream& out)
{
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty("tempo", beat.tempo);
    root->setProperty("key", beat.key);
    root->setProperty("style", beat.style);
    root->setProperty("isLoaded", beat.isLoaded);

    juce::Array<juce::var> instruments;
    for (const auto& inst : beat.instruments)
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty("name", inst.name);
        obj->setProperty("velocity", inst.velocity);

        juce::Array<juce::var> pattern;
        for (auto p : inst.pattern) pattern.add(p);
        obj->setProperty("pattern", juce::var(pattern));

        juce::Array<juce::var> notes;
        for (const auto& n : inst.notes) notes.add(n);
        obj->setProperty("notes", juce::var(notes));

        instruments.add(juce::var(obj.get()));
    }
    root->setProperty("instruments", juce::var(instruments));

    juce::String json = juce::JSON::toString(juce::var(root.get()));
    out.writeString(json);
}

void SessionState::loadState(juce::InputStream& in)
{
    juce::String json = in.readString();
    if (json.isEmpty()) return;

    juce::var data = juce::JSON::parse(json);
    if (!data.isObject()) return;

    auto* root = data.getDynamicObject();
    if (!root) return;

    Beat loaded;
    loaded.tempo    = (int)root->getProperty("tempo");
    loaded.key      = root->getProperty("key").toString();
    loaded.style    = root->getProperty("style").toString();
    loaded.isLoaded = (bool)root->getProperty("isLoaded");
    if (loaded.tempo <= 0) loaded.tempo = 120;

    auto instArray = root->getProperty("instruments");
    if (instArray.isArray())
    {
        for (const auto& instVar : *instArray.getArray())
        {
            if (!instVar.isObject()) continue;
            auto* obj = instVar.getDynamicObject();
            if (!obj) continue;

            Instrument inst;
            inst.name     = obj->getProperty("name").toString();
            inst.velocity = (int)obj->getProperty("velocity");
            if (inst.velocity <= 0) inst.velocity = 80;

            auto patVar = obj->getProperty("pattern");
            if (patVar.isArray())
                for (const auto& p : *patVar.getArray())
                    inst.pattern.add((int)p);

            auto notesVar = obj->getProperty("notes");
            if (notesVar.isArray())
                for (const auto& n : *notesVar.getArray())
                    inst.notes.add(n.toString());

            loaded.instruments.add(inst);
        }
    }

    beat = loaded;
}
