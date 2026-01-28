/*
  ==============================================================================

    OpenAIService.cpp
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#include "OpenAIService.h"

//==============================================================================
OpenAIService::OpenAIService()
{
    // Default model
    if (model.isEmpty())
        model = "gpt-4o-mini";
}

OpenAIService::~OpenAIService()
{
}

//==============================================================================
LLMService::BeatGenerationResult OpenAIService::generateBeat(const juce::String& prompt)
{
    BeatGenerationResult result;
    
    if (apiKey.isEmpty())
    {
        result.errorMessage = "OpenAI API key not set";
        return result;
    }
    
    try
    {
        // Create messages array
        juce::Array<juce::var> messages;
        
        // System message
        juce::DynamicObject::Ptr systemMsg = new juce::DynamicObject();
        systemMsg->setProperty("role", "system");
        systemMsg->setProperty("content", getSystemPromptForBeatGeneration());
        messages.add(juce::var(systemMsg.get()));
        
        // User message
        juce::DynamicObject::Ptr userMsg = new juce::DynamicObject();
        userMsg->setProperty("role", "user");
        userMsg->setProperty("content", prompt);
        messages.add(juce::var(userMsg.get()));
        
        // Create request
        juce::var request = createChatRequest(messages, true);
        
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
            result.errorMessage = "Empty response from OpenAI API";
            return result;
        }
        
        // Parse response
        juce::var responseData = parseJSONResponse(response);
        
        if (responseData.isObject())
        {
            auto* obj = responseData.getDynamicObject();
            if (obj != nullptr && obj->hasProperty("choices"))
            {
                auto choices = obj->getProperty("choices");
                if (choices.isArray())
                {
                    auto* choicesArray = choices.getArray();
                    if (choicesArray->size() > 0)
                    {
                        auto firstChoice = choicesArray->getFirst();
                        if (firstChoice.isObject())
                        {
                            auto* choiceObj = firstChoice.getDynamicObject();
                            if (choiceObj != nullptr && choiceObj->hasProperty("message"))
                            {
                                auto message = choiceObj->getProperty("message");
                                if (message.isObject())
                                {
                                    auto* msgObj = message.getDynamicObject();
                                    if (msgObj != nullptr && msgObj->hasProperty("content"))
                                    {
                                        juce::String content = msgObj->getProperty("content").toString();
                                        result.beatData = parseJSONResponse(content);
                                        result.success = true;
                                        return result;
                                    }
                                }
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
            result.errorMessage = "Failed to parse OpenAI response";
    }
    catch (const std::exception& e)
    {
        result.errorMessage = juce::String("Exception: ") + e.what();
    }
    
    return result;
}

//==============================================================================
LLMService::ComponentAdjustmentResult OpenAIService::adjustComponent(const juce::String& componentName,
                                                                      const juce::String& adjustmentPrompt,
                                                                      const juce::var& currentBeat)
{
    ComponentAdjustmentResult result;
    
    if (apiKey.isEmpty())
    {
        result.errorMessage = "OpenAI API key not set";
        return result;
    }
    
    try
    {
        // Create messages array
        juce::Array<juce::var> messages;
        
        // System message
        juce::DynamicObject::Ptr systemMsg = new juce::DynamicObject();
        systemMsg->setProperty("role", "system");
        systemMsg->setProperty("content", getSystemPromptForComponentAdjustment());
        messages.add(juce::var(systemMsg.get()));
        
        // User message with context
        juce::String userPrompt = "Current beat: " + createJSONRequest(currentBeat) + "\n";
        userPrompt += "Component to adjust: " + componentName + "\n";
        userPrompt += "User request: " + adjustmentPrompt;
        
        juce::DynamicObject::Ptr userMsg = new juce::DynamicObject();
        userMsg->setProperty("role", "user");
        userMsg->setProperty("content", userPrompt);
        messages.add(juce::var(userMsg.get()));
        
        // Create request
        juce::var request = createChatRequest(messages, true);
        
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
            result.errorMessage = "Empty response from OpenAI API";
            return result;
        }
        
        // Parse response
        juce::var responseData = parseJSONResponse(response);
        
        if (responseData.isObject())
        {
            auto* obj = responseData.getDynamicObject();
            if (obj != nullptr && obj->hasProperty("choices"))
            {
                auto choices = obj->getProperty("choices");
                if (choices.isArray())
                {
                    auto* choicesArray = choices.getArray();
                    if (choicesArray->size() > 0)
                    {
                        auto firstChoice = choicesArray->getFirst();
                        if (firstChoice.isObject())
                        {
                            auto* choiceObj = firstChoice.getDynamicObject();
                            if (choiceObj != nullptr && choiceObj->hasProperty("message"))
                            {
                                auto message = choiceObj->getProperty("message");
                                if (message.isObject())
                                {
                                    auto* msgObj = message.getDynamicObject();
                                    if (msgObj != nullptr && msgObj->hasProperty("content"))
                                    {
                                        juce::String content = msgObj->getProperty("content").toString();
                                        result.componentData = parseJSONResponse(content);
                                        result.success = true;
                                        return result;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        if (!result.success)
            result.errorMessage = "Failed to parse OpenAI response";
    }
    catch (const std::exception& e)
    {
        result.errorMessage = juce::String("Exception: ") + e.what();
    }
    
    return result;
}

//==============================================================================
LLMService::ChatResult OpenAIService::chat(const juce::String& message,
                                            const juce::Array<juce::var>& conversationHistory)
{
    ChatResult result;
    
    if (apiKey.isEmpty())
    {
        result.errorMessage = "OpenAI API key not set";
        return result;
    }
    
    try
    {
        // Create messages array
        juce::Array<juce::var> messages;
        
        // System message
        juce::DynamicObject::Ptr systemMsg = new juce::DynamicObject();
        systemMsg->setProperty("role", "system");
        systemMsg->setProperty("content", getSystemPromptForChat());
        messages.add(juce::var(systemMsg.get()));
        
        // Add conversation history
        for (const auto& historyMsg : conversationHistory)
        {
            messages.add(historyMsg);
        }
        
        // User message
        juce::DynamicObject::Ptr userMsg = new juce::DynamicObject();
        userMsg->setProperty("role", "user");
        userMsg->setProperty("content", message);
        messages.add(juce::var(userMsg.get()));
        
        // Create request
        juce::var request = createChatRequest(messages, false);
        
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
            result.errorMessage = "Empty response from OpenAI API";
            return result;
        }
        
        // Parse response
        juce::var responseData = parseJSONResponse(response);
        
        if (responseData.isObject())
        {
            auto* obj = responseData.getDynamicObject();
            if (obj != nullptr && obj->hasProperty("choices"))
            {
                auto choices = obj->getProperty("choices");
                if (choices.isArray())
                {
                    auto* choicesArray = choices.getArray();
                    if (choicesArray->size() > 0)
                    {
                        auto firstChoice = choicesArray->getFirst();
                        if (firstChoice.isObject())
                        {
                            auto* choiceObj = firstChoice.getDynamicObject();
                            if (choiceObj != nullptr && choiceObj->hasProperty("message"))
                            {
                                auto message = choiceObj->getProperty("message");
                                if (message.isObject())
                                {
                                    auto* msgObj = message.getDynamicObject();
                                    if (msgObj != nullptr && msgObj->hasProperty("content"))
                                    {
                                        result.message = msgObj->getProperty("content").toString();
                                        result.success = true;
                                        return result;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        if (!result.success)
            result.errorMessage = "Failed to parse OpenAI response";
    }
    catch (const std::exception& e)
    {
        result.errorMessage = juce::String("Exception: ") + e.what();
    }
    
    return result;
}

//==============================================================================
juce::StringPairArray OpenAIService::createHeaders()
{
    juce::StringPairArray headers;
    headers.set("Content-Type", "application/json");
    headers.set("Authorization", "Bearer " + apiKey);
    return headers;
}

//==============================================================================
juce::var OpenAIService::createChatRequest(const juce::Array<juce::var>& messages, bool useJSONMode)
{
    juce::DynamicObject::Ptr request = new juce::DynamicObject();
    request->setProperty("model", model);
    request->setProperty("messages", juce::var(messages));
    request->setProperty("temperature", temperature);
    
    if (useJSONMode)
    {
        juce::DynamicObject::Ptr responseFormat = new juce::DynamicObject();
        responseFormat->setProperty("type", "json_object");
        request->setProperty("response_format", juce::var(responseFormat.get()));
    }
    
    return juce::var(request.get());
}

//==============================================================================
juce::String OpenAIService::getSystemPromptForBeatGeneration()
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
juce::String OpenAIService::getSystemPromptForComponentAdjustment()
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
juce::String OpenAIService::getSystemPromptForChat()
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

