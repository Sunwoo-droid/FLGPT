/*
  ==============================================================================

    AnthropicService.cpp
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#include "AnthropicService.h"

//==============================================================================
AnthropicService::AnthropicService()
{
    // Default model
    if (model.isEmpty())
        model = "claude-3-5-sonnet-20241022";
}

AnthropicService::~AnthropicService()
{
}

//==============================================================================
LLMService::BeatGenerationResult AnthropicService::generateBeat(const juce::String& prompt)
{
    BeatGenerationResult result;
    
    if (apiKey.isEmpty())
    {
        result.errorMessage = "Anthropic API key not set";
        return result;
    }
    
    try
    {
        juce::String systemPrompt = getSystemPromptForBeatGeneration();
        juce::var request = createMessageRequest(prompt, systemPrompt);
        
        // Perform HTTP request
        juce::StringPairArray headers = createHeaders();
        juce::String requestBody = createJSONRequest(request);
        
        // Use JUCE's URL class for HTTP POST
        juce::URL url(API_URL);
        url = url.withPOSTData(requestBody);
        
        // Build header string
        juce::String headerString;
        for (int i = 0; i < headers.size(); ++i)
        {
            headerString += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i];
            if (i < headers.size() - 1)
                headerString += "\r\n";
        }
        
        std::unique_ptr<juce::InputStream> stream = url.createInputStream(
            juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                .withConnectionTimeoutMs(30000)
                .withHttpRequestCmd("POST")
                .withExtraHeaders(headerString)
        );
        
        juce::String response;
        if (stream != nullptr)
            response = stream->readEntireStreamAsString();
        
        if (response.isEmpty())
        {
            result.errorMessage = "Empty response from Anthropic API";
            return result;
        }
        
        // Parse response
        juce::var responseData = parseJSONResponse(response);
        
        if (responseData.isObject())
        {
            auto* obj = responseData.getDynamicObject();
            if (obj != nullptr && obj->hasProperty("content"))
            {
                auto content = obj->getProperty("content");
                if (content.isArray())
                {
                    auto* contentArray = content.getArray();
                    if (contentArray->size() > 0)
                    {
                        auto firstContent = contentArray->getFirst();
                        if (firstContent.isObject())
                        {
                            auto* contentObj = firstContent.getDynamicObject();
                            if (contentObj != nullptr && contentObj->hasProperty("text"))
                            {
                                juce::String text = contentObj->getProperty("text").toString();
                                result.beatData = parseJSONResponse(text);
                                result.success = true;
                                return result;
                            }
                        }
                    }
                }
            }
            
            // Check for error
            if (obj->hasProperty("error"))
            {
                auto error = obj->getProperty("error");
                if (error.isObject())
                {
                    auto* errorObj = error.getDynamicObject();
                    if (errorObj != nullptr && errorObj->hasProperty("message"))
                        result.errorMessage = errorObj->getProperty("message").toString();
                }
            }
        }
        
        if (!result.success)
            result.errorMessage = "Failed to parse Anthropic response";
    }
    catch (const std::exception& e)
    {
        result.errorMessage = juce::String("Exception: ") + e.what();
    }
    
    return result;
}

//==============================================================================
LLMService::ComponentAdjustmentResult AnthropicService::adjustComponent(const juce::String& componentName,
                                                                           const juce::String& adjustmentPrompt,
                                                                           const juce::var& currentBeat)
{
    ComponentAdjustmentResult result;
    
    if (apiKey.isEmpty())
    {
        result.errorMessage = "Anthropic API key not set";
        return result;
    }
    
    try
    {
        juce::String userPrompt = "Current beat: " + createJSONRequest(currentBeat) + "\n";
        userPrompt += "Component to adjust: " + componentName + "\n";
        userPrompt += "User request: " + adjustmentPrompt;
        
        juce::String systemPrompt = getSystemPromptForComponentAdjustment();
        juce::var request = createMessageRequest(userPrompt, systemPrompt);
        
        // Perform HTTP request
        juce::StringPairArray headers = createHeaders();
        juce::String requestBody = createJSONRequest(request);
        
        // Use JUCE's URL class for HTTP POST
        juce::URL url(API_URL);
        url = url.withPOSTData(requestBody);
        
        // Build header string
        juce::String headerString;
        for (int i = 0; i < headers.size(); ++i)
        {
            headerString += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i];
            if (i < headers.size() - 1)
                headerString += "\r\n";
        }
        
        std::unique_ptr<juce::InputStream> stream = url.createInputStream(
            juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                .withConnectionTimeoutMs(30000)
                .withHttpRequestCmd("POST")
                .withExtraHeaders(headerString)
        );
        
        juce::String response;
        if (stream != nullptr)
            response = stream->readEntireStreamAsString();
        
        if (response.isEmpty())
        {
            result.errorMessage = "Empty response from Anthropic API";
            return result;
        }
        
        // Parse response
        juce::var responseData = parseJSONResponse(response);
        
        if (responseData.isObject())
        {
            auto* obj = responseData.getDynamicObject();
            if (obj != nullptr && obj->hasProperty("content"))
            {
                auto content = obj->getProperty("content");
                if (content.isArray())
                {
                    auto* contentArray = content.getArray();
                    if (contentArray->size() > 0)
                    {
                        auto firstContent = contentArray->getFirst();
                        if (firstContent.isObject())
                        {
                            auto* contentObj = firstContent.getDynamicObject();
                            if (contentObj != nullptr && contentObj->hasProperty("text"))
                            {
                                juce::String text = contentObj->getProperty("text").toString();
                                result.componentData = parseJSONResponse(text);
                                result.success = true;
                                return result;
                            }
                        }
                    }
                }
            }
        }
        
        if (!result.success)
            result.errorMessage = "Failed to parse Anthropic response";
    }
    catch (const std::exception& e)
    {
        result.errorMessage = juce::String("Exception: ") + e.what();
    }
    
    return result;
}

//==============================================================================
LLMService::ChatResult AnthropicService::chat(const juce::String& message,
                                               const juce::Array<juce::var>& conversationHistory)
{
    ChatResult result;
    
    if (apiKey.isEmpty())
    {
        result.errorMessage = "Anthropic API key not set";
        return result;
    }
    
    try
    {
        // Build conversation context
        juce::String conversationText;
        for (const auto& historyMsg : conversationHistory)
        {
            if (historyMsg.isObject())
            {
                auto* msgObj = historyMsg.getDynamicObject();
                if (msgObj != nullptr)
                {
                    juce::String role = msgObj->getProperty("role").toString();
                    juce::String content = msgObj->getProperty("content").toString();
                    conversationText += role + ": " + content + "\n\n";
                }
            }
        }
        conversationText += "User: " + message;
        
        juce::String systemPrompt = getSystemPromptForChat();
        juce::var request = createMessageRequest(conversationText, systemPrompt);
        
        // Perform HTTP request
        juce::StringPairArray headers = createHeaders();
        juce::String requestBody = createJSONRequest(request);
        
        // Use JUCE's URL class for HTTP POST
        juce::URL url(API_URL);
        url = url.withPOSTData(requestBody);
        
        // Build header string
        juce::String headerString;
        for (int i = 0; i < headers.size(); ++i)
        {
            headerString += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i];
            if (i < headers.size() - 1)
                headerString += "\r\n";
        }
        
        std::unique_ptr<juce::InputStream> stream = url.createInputStream(
            juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                .withConnectionTimeoutMs(30000)
                .withHttpRequestCmd("POST")
                .withExtraHeaders(headerString)
        );
        
        juce::String response;
        if (stream != nullptr)
            response = stream->readEntireStreamAsString();
        
        if (response.isEmpty())
        {
            result.errorMessage = "Empty response from Anthropic API";
            return result;
        }
        
        // Parse response
        juce::var responseData = parseJSONResponse(response);
        
        if (responseData.isObject())
        {
            auto* obj = responseData.getDynamicObject();
            if (obj != nullptr && obj->hasProperty("content"))
            {
                auto content = obj->getProperty("content");
                if (content.isArray())
                {
                    auto* contentArray = content.getArray();
                    if (contentArray->size() > 0)
                    {
                        auto firstContent = contentArray->getFirst();
                        if (firstContent.isObject())
                        {
                            auto* contentObj = firstContent.getDynamicObject();
                            if (contentObj != nullptr && contentObj->hasProperty("text"))
                            {
                                result.message = contentObj->getProperty("text").toString();
                                result.success = true;
                                return result;
                            }
                        }
                    }
                }
            }
        }
        
        if (!result.success)
            result.errorMessage = "Failed to parse Anthropic response";
    }
    catch (const std::exception& e)
    {
        result.errorMessage = juce::String("Exception: ") + e.what();
    }
    
    return result;
}

//==============================================================================
juce::StringPairArray AnthropicService::createHeaders()
{
    juce::StringPairArray headers;
    headers.set("Content-Type", "application/json");
    headers.set("x-api-key", apiKey);
    headers.set("anthropic-version", "2023-06-01");
    return headers;
}

//==============================================================================
juce::var AnthropicService::createMessageRequest(const juce::String& userMessage, const juce::String& systemPrompt)
{
    juce::DynamicObject::Ptr request = new juce::DynamicObject();
    request->setProperty("model", model);
    request->setProperty("max_tokens", 4096);
    request->setProperty("system", systemPrompt);
    
    juce::Array<juce::var> messages;
    juce::DynamicObject::Ptr userMsg = new juce::DynamicObject();
    userMsg->setProperty("role", "user");
    userMsg->setProperty("content", userMessage);
    messages.add(juce::var(userMsg.get()));
    
    request->setProperty("messages", juce::var(messages));
    
    return juce::var(request.get());
}

//==============================================================================
juce::String AnthropicService::getSystemPromptForBeatGeneration()
{
    return R"(You are an expert music production assistant specializing in beat creation. 
Analyze the user's beat request and return a structured JSON response with the following format:
{
  "tempo": 120,
  "timeSignature": "4/4",
  "key": "C minor",
  "style": "description of style",
  "instruments": [
    {
      "name": "Kick",
      "pattern": [1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0],
      "notes": ["C2"],
      "velocity": 100
    },
    {
      "name": "Snare",
      "pattern": [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0],
      "notes": ["D2"],
      "velocity": 80
    }
  ],
  "description": "detailed description of the beat"
}

Pattern arrays use 1 for hit and 0 for rest, representing 16th note subdivisions.
Return ONLY valid JSON, no additional text.)";
}

//==============================================================================
juce::String AnthropicService::getSystemPromptForComponentAdjustment()
{
    return R"(You are a music production assistant. The user wants to adjust a specific component of their beat.
Given the current beat structure and the user's request, return ONLY a JSON object with the updated component:
{
  "name": "ComponentName",
  "pattern": [array of 1s and 0s],
  "notes": ["note1", "note2"],
  "velocity": 80
}

Return ONLY valid JSON, no additional text.)";
}

//==============================================================================
juce::String AnthropicService::getSystemPromptForChat()
{
    return R"(You are an expert music production assistant helping users create beats in FL Studio.
You understand music theory, drum patterns, sound design, and can help with:
- Beat creation and arrangement
- Sound selection and synthesis
- Mixing and effects
- Music theory questions
- FL Studio specific workflows

Be helpful, concise, and musically accurate.)";
}

