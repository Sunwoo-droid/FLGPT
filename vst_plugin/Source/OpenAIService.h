#pragma once

#include <juce_core/juce_core.h>
#include "SessionState.h"

class OpenAIService
{
public:
    struct Response
    {
        bool success = false;
        juce::String message;
        // "generate_beat" | "update_component" | "none"
        juce::String action;
        SessionState::Beat beat;
        juce::String componentName;
        SessionState::Instrument component;
        juce::String error;
    };

    void setApiKey(const juce::String& key) { apiKey = key; }
    const juce::String& getApiKey() const   { return apiKey; }
    bool hasApiKey() const                  { return apiKey.isNotEmpty(); }

    // One call handles everything: new beats, component edits, and conversation.
    // The LLM decides which action to take based on the user message + current beat context.
    Response sendMessage(const juce::String& userMessage,
                         const SessionState::Beat& currentBeat);

private:
    juce::String apiKey;
    juce::String model = "gpt-4o-mini";

    static constexpr const char* API_URL = "https://api.openai.com/v1/chat/completions";

    juce::String buildSystemPrompt() const;
    juce::String beatToJson(const SessionState::Beat& beat) const;
    SessionState::Beat       parseBeat(const juce::var& obj) const;
    SessionState::Instrument parseInstrument(const juce::var& obj) const;
};
