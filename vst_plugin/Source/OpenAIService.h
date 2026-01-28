/*
  ==============================================================================

    OpenAIService.h
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#pragma once

#include "LLMService.h"

//==============================================================================
/**
    OpenAI API implementation for GPT models.
*/
class OpenAIService : public LLMService
{
public:
    //==============================================================================
    OpenAIService();
    ~OpenAIService() override;
    
    //==============================================================================
    BeatGenerationResult generateBeat(const juce::String& prompt) override;
    ComponentAdjustmentResult adjustComponent(const juce::String& componentName,
                                               const juce::String& adjustmentPrompt,
                                               const juce::var& currentBeat) override;
    ChatResult chat(const juce::String& message,
                   const juce::Array<juce::var>& conversationHistory = {}) override;

private:
    //==============================================================================
    static constexpr const char* API_URL = "https://api.openai.com/v1/chat/completions";
    
    juce::StringPairArray createHeaders();
    juce::var createChatRequest(const juce::Array<juce::var>& messages, bool useJSONMode = false);
    juce::String getSystemPromptForBeatGeneration();
    juce::String getSystemPromptForComponentAdjustment();
    juce::String getSystemPromptForChat();
};

