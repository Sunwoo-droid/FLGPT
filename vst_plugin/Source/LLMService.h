/*
  ==============================================================================

    LLMService.h
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>

//==============================================================================
/**
    Base class for LLM API services (OpenAI, Anthropic, etc.)
    Provides unified interface for beat generation and component adjustment.
*/
class LLMService
{
public:
    //==============================================================================
    LLMService();
    virtual ~LLMService();
    
    //==============================================================================
    // Result structure for API calls
    struct BeatGenerationResult
    {
        bool success = false;
        juce::var beatData;  // JSON beat data
        juce::String errorMessage;
    };
    
    struct ComponentAdjustmentResult
    {
        bool success = false;
        juce::var componentData;  // JSON component data
        juce::String errorMessage;
    };
    
    struct ChatResult
    {
        bool success = false;
        juce::String message;
        juce::String errorMessage;
    };
    
    //==============================================================================
    // Pure virtual methods to be implemented by subclasses
    virtual BeatGenerationResult generateBeat(const juce::String& prompt) = 0;
    virtual ComponentAdjustmentResult adjustComponent(const juce::String& componentName,
                                                       const juce::String& adjustmentPrompt,
                                                       const juce::var& currentBeat) = 0;
    virtual ChatResult chat(const juce::String& message,
                           const juce::Array<juce::var>& conversationHistory = {}) = 0;
    
    //==============================================================================
    // Configuration
    virtual void setAPIKey(const juce::String& apiKey) { this->apiKey = apiKey; }
    virtual juce::String getAPIKey() const { return apiKey; }
    
    virtual void setModel(const juce::String& model) { this->model = model; }
    virtual juce::String getModel() const { return model; }
    
    virtual void setTemperature(float temperature) { this->temperature = temperature; }
    virtual float getTemperature() const { return temperature; }
    
    //==============================================================================
    // Factory method
    static std::unique_ptr<LLMService> createService(const juce::String& serviceType);

protected:
    //==============================================================================
    juce::String apiKey;
    juce::String model;
    float temperature = 0.7f;
    
    // Helper methods for HTTP requests
    juce::String performHTTPRequest(const juce::String& url,
                                    const juce::String& method,
                                    const juce::String& requestBody,
                                    const juce::StringPairArray& headers);
    
    // JSON parsing helpers
    juce::var parseJSONResponse(const juce::String& jsonString);
    juce::String createJSONRequest(const juce::var& data);
};

