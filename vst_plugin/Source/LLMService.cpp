/*
  ==============================================================================

    LLMService.cpp
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#include "LLMService.h"
#include "OpenAIService.h"
#include "AnthropicService.h"

//==============================================================================
LLMService::LLMService()
{
}

LLMService::~LLMService()
{
}

//==============================================================================
std::unique_ptr<LLMService> LLMService::createService(const juce::String& serviceType)
{
    if (serviceType == "OpenAI" || serviceType == "openai")
        return std::make_unique<OpenAIService>();
    else if (serviceType == "Anthropic" || serviceType == "anthropic" || serviceType == "Claude")
        return std::make_unique<AnthropicService>();
    
    // Default to OpenAI
    return std::make_unique<OpenAIService>();
}

//==============================================================================
juce::String LLMService::performHTTPRequest(const juce::String& url,
                                             const juce::String& method,
                                             const juce::String& requestBody,
                                             const juce::StringPairArray& headers)
{
    juce::URL requestURL(url);
    
    // Set POST data on the URL if method is POST
    if (method == "POST")
    {
        requestURL = requestURL.withPOSTData(requestBody);
    }
    
    // Build custom headers string
    juce::String extraHeaders;
    for (int i = 0; i < headers.size(); ++i)
    {
        extraHeaders += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i] + "\r\n";
    }
    
    // Create input stream options (chain methods since options are immutable)
    auto baseOptions = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
        .withConnectionTimeoutMs(30000)
        .withExtraHeaders(extraHeaders);
    
    // Add HTTP method if POST
    auto options = (method == "POST") 
        ? baseOptions.withHttpRequestCmd("POST")
        : baseOptions;
    
    auto stream = requestURL.createInputStream(options);
    
    if (stream == nullptr)
        return {};
    
    return stream->readEntireStreamAsString();
}

//==============================================================================
juce::var LLMService::parseJSONResponse(const juce::String& jsonString)
{
    return juce::JSON::parse(jsonString);
}

//==============================================================================
juce::String LLMService::createJSONRequest(const juce::var& data)
{
    return juce::JSON::toString(data, true);
}

