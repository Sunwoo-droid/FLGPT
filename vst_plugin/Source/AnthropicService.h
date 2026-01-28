/*
  ==============================================================================

    AnthropicService.h
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#pragma once

#include "LLMService.h"

//==============================================================================
/**
    Anthropic API implementation for Claude models.
*/
class AnthropicService : public LLMService
{
public:
    //==============================================================================
    AnthropicService();
    ~AnthropicService() override;
    
    //==============================================================================
    BeatGenerationResult generateBeat(const juce::String& prompt) override;
    ComponentAdjustmentResult adjustComponent(const juce::String& componentName,
                                               const juce::String& adjustmentPrompt,
                                               const juce::var& currentBeat) override;
    ChatResult chat(const juce::String& message,
                   const juce::Array<juce::var>& conversationHistory = {}) override;

private:
    //==============================================================================
    static constexpr const char* API_URL = "https://api.anthropic.com/v1/messages";
    
    juce::StringPairArray createHeaders();
    juce::var createMessageRequest(const juce::String& userMessage, const juce::String& systemPrompt);
    juce::String getSystemPromptForBeatGeneration();
    juce::String getSystemPromptForComponentAdjustment();
    juce::String getSystemPromptForChat();
};

