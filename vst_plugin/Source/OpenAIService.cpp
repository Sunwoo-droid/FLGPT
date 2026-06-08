#include "OpenAIService.h"

OpenAIService::Response OpenAIService::sendMessage(const juce::String& userMessage,
                                                     const SessionState::Beat& currentBeat)
{
    Response resp;

    if (!hasApiKey())
    {
        resp.error = "No API key set. Click Settings to add your OpenAI API key.";
        return resp;
    }

    // Build messages array
    juce::Array<juce::var> messages;

    {
        juce::DynamicObject::Ptr sys = new juce::DynamicObject();
        sys->setProperty("role", "system");
        sys->setProperty("content", buildSystemPrompt());
        messages.add(juce::var(sys.get()));
    }

    {
        juce::String userContent = userMessage;
        if (currentBeat.isLoaded)
            userContent += "\n\nCurrent beat:\n" + beatToJson(currentBeat);

        juce::DynamicObject::Ptr usr = new juce::DynamicObject();
        usr->setProperty("role", "user");
        usr->setProperty("content", userContent);
        messages.add(juce::var(usr.get()));
    }

    // Build request body
    juce::DynamicObject::Ptr reqObj = new juce::DynamicObject();
    reqObj->setProperty("model", model);
    reqObj->setProperty("messages", juce::var(messages));
    reqObj->setProperty("temperature", 0.7);

    juce::DynamicObject::Ptr respFmt = new juce::DynamicObject();
    respFmt->setProperty("type", "json_object");
    reqObj->setProperty("response_format", juce::var(respFmt.get()));

    const juce::String body    = juce::JSON::toString(juce::var(reqObj.get()));
    const juce::String headers = "Content-Type: application/json\r\nAuthorization: Bearer " + apiKey;

    juce::URL url(API_URL);
    url = url.withPOSTData(body);

    std::unique_ptr<juce::InputStream> stream = url.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
            .withConnectionTimeoutMs(30000)
            .withHttpRequestCmd("POST")
            .withExtraHeaders(headers)
    );

    if (!stream)
    {
        resp.error = "Network request failed. Check your internet connection.";
        return resp;
    }

    const juce::String responseText = stream->readEntireStreamAsString();
    const juce::var   responseData  = juce::JSON::parse(responseText);

    if (!responseData.isObject())
    {
        resp.error = "Invalid response from OpenAI.";
        return resp;
    }

    auto* resObj = responseData.getDynamicObject();

    if (resObj->hasProperty("error"))
    {
        auto errVar = resObj->getProperty("error");
        if (errVar.isObject())
        {
            auto* errObj = errVar.getDynamicObject();
            resp.error = errObj ? errObj->getProperty("message").toString() : "OpenAI API error";
        }
        else
        {
            resp.error = "OpenAI API error";
        }
        return resp;
    }

    // Extract content from choices[0].message.content
    juce::String content;
    auto choices = resObj->getProperty("choices");
    if (choices.isArray() && choices.getArray()->size() > 0)
    {
        auto firstChoice = (*choices.getArray())[0];
        if (firstChoice.isObject())
        {
            auto* choiceObj = firstChoice.getDynamicObject();
            if (choiceObj)
            {
                auto msgVar = choiceObj->getProperty("message");
                if (msgVar.isObject())
                {
                    auto* msgObj = msgVar.getDynamicObject();
                    if (msgObj) content = msgObj->getProperty("content").toString();
                }
            }
        }
    }

    if (content.isEmpty())
    {
        resp.error = "Empty response from OpenAI.";
        return resp;
    }

    // Parse the LLM's JSON response
    juce::var parsed = juce::JSON::parse(content);
    if (!parsed.isObject())
    {
        resp.error = "LLM returned invalid JSON: " + content.substring(0, 100);
        return resp;
    }

    auto* parsedObj = parsed.getDynamicObject();
    resp.message = parsedObj->getProperty("message").toString();
    resp.action  = parsedObj->getProperty("action").toString();
    resp.success = true;

    if (resp.action == "generate_beat" && parsedObj->hasProperty("beat"))
    {
        resp.beat = parseBeat(parsedObj->getProperty("beat"));
    }
    else if (resp.action == "update_component" && parsedObj->hasProperty("component"))
    {
        resp.componentName = parsedObj->getProperty("component_name").toString();
        resp.component     = parseInstrument(parsedObj->getProperty("component"));
    }

    return resp;
}

juce::String OpenAIService::buildSystemPrompt() const
{
    return R"(You are a beat-making AI for FL Studio. Help users create and edit drum/instrument patterns using natural language.

ALWAYS respond with valid JSON only — no extra text, no markdown code blocks. Format:

For a new beat:
{"message":"...","action":"generate_beat","beat":{...}}

For editing one instrument:
{"message":"...","action":"update_component","component_name":"Hi-Hat","component":{...}}

For conversation with no beat change:
{"message":"...","action":"none"}

Beat format:
{
  "tempo": 90,
  "key": "C minor",
  "style": "lo-fi hip hop",
  "instruments": [
    {"name":"Kick",   "pattern":[1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0],"notes":["C2"], "velocity":100},
    {"name":"Snare",  "pattern":[0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0],"notes":["D2"], "velocity":80},
    {"name":"Hi-Hat", "pattern":[1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0],"notes":["F#2"],"velocity":60}
  ]
}

Component format (for update_component):
{"name":"Hi-Hat","pattern":[1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1],"notes":["F#2"],"velocity":80}

Rules:
- pattern is EXACTLY 16 integers (0 or 1), representing 16th-note steps in one bar
- notes use scientific pitch: C2, D#3, F#2, etc.
- velocity: 1-127
- Standard GM drum notes: Kick=C2(36), Snare=D2(38), Hi-Hat=F#2(42), Open Hi-Hat=A#2(46), Clap=D#2(39), Crash=A2(49)
- Use "generate_beat" when user wants a completely new beat from scratch
- Use "update_component" when user mentions changing a specific instrument while keeping others
- Use "none" for questions or chitchat with no beat modification needed
- Lo-fi: 70-95 BPM, swung hi-hats, sparse kick/snare
- Trap: 130-150 BPM, triplet hi-hats, heavy 808 kick
- House: 120-130 BPM, four-on-the-floor kick, open hi-hat on offbeats
- Drill: 140-150 BPM, rolling hi-hats, syncopated snare
- Make patterns musically authentic to the requested style)";
}

juce::String OpenAIService::beatToJson(const SessionState::Beat& beat) const
{
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("tempo", beat.tempo);
    obj->setProperty("key", beat.key);
    obj->setProperty("style", beat.style);

    juce::Array<juce::var> instruments;
    for (const auto& inst : beat.instruments)
    {
        juce::DynamicObject::Ptr instObj = new juce::DynamicObject();
        instObj->setProperty("name", inst.name);
        instObj->setProperty("velocity", inst.velocity);

        juce::Array<juce::var> pattern;
        for (auto p : inst.pattern) pattern.add(p);
        instObj->setProperty("pattern", juce::var(pattern));

        juce::Array<juce::var> notes;
        for (const auto& n : inst.notes) notes.add(n);
        instObj->setProperty("notes", juce::var(notes));

        instruments.add(juce::var(instObj.get()));
    }
    obj->setProperty("instruments", juce::var(instruments));

    return juce::JSON::toString(juce::var(obj.get()));
}

SessionState::Beat OpenAIService::parseBeat(const juce::var& beatVar) const
{
    SessionState::Beat beat;
    if (!beatVar.isObject()) return beat;

    auto* obj = beatVar.getDynamicObject();
    if (!obj) return beat;

    beat.tempo = (int)obj->getProperty("tempo");
    if (beat.tempo <= 0) beat.tempo = 120;
    beat.key   = obj->getProperty("key").toString();
    beat.style = obj->getProperty("style").toString();

    auto instArray = obj->getProperty("instruments");
    if (instArray.isArray())
        for (const auto& instVar : *instArray.getArray())
            beat.instruments.add(parseInstrument(instVar));

    return beat;
}

SessionState::Instrument OpenAIService::parseInstrument(const juce::var& instVar) const
{
    SessionState::Instrument inst;
    if (!instVar.isObject()) return inst;

    auto* obj = instVar.getDynamicObject();
    if (!obj) return inst;

    inst.name     = obj->getProperty("name").toString();
    inst.velocity = (int)obj->getProperty("velocity");
    if (inst.velocity <= 0) inst.velocity = 80;

    auto patVar = obj->getProperty("pattern");
    if (patVar.isArray())
    {
        for (const auto& p : *patVar.getArray())
            inst.pattern.add((int)p);
        // Ensure exactly 16 steps
        while (inst.pattern.size() < 16) inst.pattern.add(0);
        if (inst.pattern.size() > 16)    inst.pattern.resize(16);
    }

    auto notesVar = obj->getProperty("notes");
    if (notesVar.isArray())
        for (const auto& n : *notesVar.getArray())
            inst.notes.add(n.toString());

    if (inst.notes.isEmpty()) inst.notes.add("C3");

    return inst;
}
