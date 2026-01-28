/*
  ==============================================================================

    ChatComponent.cpp
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#include "ChatComponent.h"

//==============================================================================
ChatComponent::ChatComponent()
{
    // Setup input field
    inputField.setMultiLine(false);
    inputField.setReturnKeyStartsNewLine(false);
    inputField.setFont(juce::Font(juce::FontOptions(14.0f)));
    inputField.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    inputField.setColour(juce::TextEditor::textColourId, juce::Colour(0xffffffff));
    inputField.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff444444));
    inputField.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff4a9eff));
    inputField.addListener(this);
    addAndMakeVisible(inputField);
    
    // Setup send button
    sendButton.setButtonText("Send");
    sendButton.addListener(this);
    sendButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a9eff));
    sendButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffffff));
    addAndMakeVisible(sendButton);
    
    // Setup settings button
    settingsButton.setButtonText("Settings");
    settingsButton.addListener(this);
    settingsButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff6c757d));
    settingsButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffffff));
    addAndMakeVisible(settingsButton);
    
    // Setup chat viewport
    chatViewport.setViewedComponent(&chatContent, false);
    chatViewport.setScrollBarsShown(true, false);
    addAndMakeVisible(chatViewport);
    
    // Initial welcome message
    addMessage("Welcome to FLGPT! Try asking me to create a beat, or ask a music production question.", false);
}

ChatComponent::~ChatComponent()
{
    if (sessionState != nullptr)
        sessionState->removeListener(this);
    
    if (llmThread != nullptr && llmThread->isThreadRunning())
    {
        llmThread->stopThread(5000);
    }
}

//==============================================================================
void ChatComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
    
    // Draw chat area background
    g.setColour(juce::Colour(0xff252525));
    g.fillRect(chatViewport.getBounds());
    
    // Draw messages
    int y = 10;
    int width = chatContent.getWidth() - 20;
    
    for (const auto& message : messages)
    {
        int height = calculateMessageHeight(message, width);
        drawMessage(g, message, y, width);
        y += height + 10;
    }
    
    // Update content height
    chatContent.setSize(chatContent.getWidth(), y + 10);
}

//==============================================================================
void ChatComponent::resized()
{
    auto bounds = getLocalBounds();
    
    // Settings button (top right)
    settingsButton.setBounds(bounds.removeFromTop(30).removeFromRight(100).reduced(5));
    
    // Chat viewport (main area)
    chatViewport.setBounds(bounds.removeFromTop(bounds.getHeight() - 50));
    chatContent.setSize(chatViewport.getWidth() - 20, chatContent.getHeight());
    
    // Input area (bottom)
    auto inputArea = bounds.reduced(5);
    sendButton.setBounds(inputArea.removeFromRight(80));
    inputArea.removeFromRight(5);
    inputField.setBounds(inputArea);
}

//==============================================================================
void ChatComponent::buttonClicked(juce::Button* button)
{
    if (button == &sendButton)
    {
        sendMessage();
    }
    else if (button == &settingsButton)
    {
        // TODO: Open settings dialog
        addMessage("Settings dialog coming soon!", false);
    }
}

//==============================================================================
void ChatComponent::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    if (&editor == &inputField)
    {
        sendMessage();
    }
}

//==============================================================================
void ChatComponent::beatDataChanged()
{
    if (sessionState != nullptr)
    {
        displayBeatInfo(sessionState->getCurrentBeat());
    }
    updateChatDisplay();
}

//==============================================================================
void ChatComponent::componentUpdated(const juce::String& componentName)
{
    addMessage("Component '" + componentName + "' has been updated.", false);
    updateChatDisplay();
}

//==============================================================================
void ChatComponent::setLLMService(LLMService* service)
{
    llmService = service;
}

//==============================================================================
void ChatComponent::setSessionState(SessionState* state)
{
    if (sessionState != nullptr)
        sessionState->removeListener(this);
    
    sessionState = state;
    
    if (sessionState != nullptr)
        sessionState->addListener(this);
}

//==============================================================================
void ChatComponent::sendMessage()
{
    juce::String message = inputField.getText().trim();
    if (message.isEmpty() || isLoading)
        return;
    
    inputField.setText("");
    addMessage(message, true);
    
    // Determine message type and handle accordingly
    if (isBeatRequest(message))
    {
        handleBeatGeneration(message);
    }
    else
    {
        handleGeneralChat(message);
    }
}

//==============================================================================
void ChatComponent::addMessage(const juce::String& text, bool isUser)
{
    ChatMessage msg;
    msg.text = text;
    msg.isUser = isUser;
    msg.timestamp = juce::Time::getCurrentTime();
    
    messages.add(msg);
    
    // Add to conversation history for LLM
    if (llmService != nullptr)
    {
        juce::DynamicObject::Ptr msgObj = new juce::DynamicObject();
        msgObj->setProperty("role", isUser ? "user" : "assistant");
        msgObj->setProperty("content", text);
        conversationHistory.add(juce::var(msgObj.get()));
    }
    
    updateChatDisplay();
}

//==============================================================================
void ChatComponent::displayBeatInfo(const SessionState::BeatData& beat)
{
    juce::String info = "Beat Generated!\n\n";
    info += "Tempo: " + juce::String(beat.tempo) + " BPM\n";
    info += "Key: " + beat.key + "\n";
    info += "Time Signature: " + beat.timeSignature + "\n";
    info += "Style: " + beat.style + "\n\n";
    info += "Instruments: ";
    
    for (int i = 0; i < beat.instruments.size(); ++i)
    {
        if (i > 0) info += ", ";
        info += beat.instruments[i].name;
    }
    
    if (!beat.description.isEmpty())
    {
        info += "\n\n" + beat.description;
    }
    
    addMessage(info, false);
}

//==============================================================================
void ChatComponent::updateChatDisplay()
{
    chatContent.repaint();
    chatViewport.setViewPositionProportionately(0.0, 1.0); // Scroll to bottom
}

//==============================================================================
void ChatComponent::handleBeatGeneration(const juce::String& prompt)
{
    if (llmService == nullptr)
    {
        showError("LLM service not configured. Please set API key in settings.");
        return;
    }
    
    if (isLoading)
        return;
    
    showLoading(true);
    
    // Create and start background thread
    llmThread = std::make_unique<LLMThread>(this);
    llmThread->prompt = prompt;
    llmThread->requestType = 0;
    llmThread->startThread();
}

//==============================================================================
void ChatComponent::handleComponentAdjustment(const juce::String& componentName, const juce::String& adjustment)
{
    if (llmService == nullptr || sessionState == nullptr)
    {
        showError("Service or session state not available.");
        return;
    }
    
    if (isLoading)
        return;
    
    showLoading(true);
    
    juce::var currentBeat;
    // Convert session state to JSON for LLM
    auto& beat = sessionState->getCurrentBeat();
    juce::DynamicObject::Ptr beatObj = new juce::DynamicObject();
    beatObj->setProperty("tempo", beat.tempo);
    beatObj->setProperty("timeSignature", beat.timeSignature);
    beatObj->setProperty("key", beat.key);
    beatObj->setProperty("style", beat.style);
    
    juce::Array<juce::var> instruments;
    for (const auto& inst : beat.instruments)
    {
        juce::DynamicObject::Ptr instObj = new juce::DynamicObject();
        instObj->setProperty("name", inst.name);
        juce::Array<juce::var> pattern;
        for (auto p : inst.pattern)
            pattern.add(p);
        instObj->setProperty("pattern", juce::var(pattern));
        juce::Array<juce::var> notes;
        for (const auto& n : inst.notes)
            notes.add(n);
        instObj->setProperty("notes", juce::var(notes));
        instObj->setProperty("velocity", inst.velocity);
        instruments.add(juce::var(instObj.get()));
    }
    beatObj->setProperty("instruments", juce::var(instruments));
    currentBeat = juce::var(beatObj.get());
    
    // Create and start background thread
    llmThread = std::make_unique<LLMThread>(this);
    llmThread->componentName = componentName;
    llmThread->adjustment = adjustment;
    llmThread->currentBeat = currentBeat;
    llmThread->requestType = 1;
    llmThread->startThread();
}

//==============================================================================
void ChatComponent::handleGeneralChat(const juce::String& message)
{
    if (llmService == nullptr)
    {
        showError("LLM service not configured. Please set API key in settings.");
        return;
    }
    
    if (isLoading)
        return;
    
    showLoading(true);
    
    // Create and start background thread
    llmThread = std::make_unique<LLMThread>(this);
    llmThread->prompt = message;
    llmThread->requestType = 2;
    llmThread->startThread();
}

//==============================================================================
bool ChatComponent::isBeatRequest(const juce::String& prompt)
{
    juce::String lower = prompt.toLowerCase();
    return lower.contains("beat") ||
           lower.contains("create") ||
           lower.contains("make") ||
           lower.contains("generate") ||
           lower.contains("produce") ||
           lower.contains("type of");
}

//==============================================================================
void ChatComponent::showError(const juce::String& errorMessage)
{
    addMessage("Error: " + errorMessage, false);
}

//==============================================================================
void ChatComponent::showLoading(bool show)
{
    isLoading = show;
    sendButton.setEnabled(!show);
    inputField.setEnabled(!show);
    
    if (show)
    {
        addMessage("Thinking...", false);
    }
}

//==============================================================================
void ChatComponent::drawMessage(juce::Graphics& g, const ChatMessage& message, int y, int width)
{
    int padding = 10;
    int cornerRadius = 8;
    
    juce::Colour bgColour = message.isUser ? juce::Colour(0xff4a9eff) : juce::Colour(0xff2a2a2a);
    juce::Colour textColour = juce::Colour(0xffffffff);
    
    juce::Rectangle<int> messageRect;
    if (message.isUser)
    {
        messageRect = juce::Rectangle<int>(width - 200, y, 190, calculateMessageHeight(message, width));
    }
    else
    {
        messageRect = juce::Rectangle<int>(10, y, width - 220, calculateMessageHeight(message, width));
    }
    
    g.setColour(bgColour);
    g.fillRoundedRectangle(messageRect.toFloat(), cornerRadius);
    
    g.setColour(textColour);
    g.setFont(juce::Font(juce::FontOptions(14.0f)));
    g.drawText(message.text, messageRect.reduced(padding), juce::Justification::topLeft, true);
}

//==============================================================================
int ChatComponent::calculateMessageHeight(const ChatMessage& message, int width)
{
    int maxWidth = message.isUser ? 190 : (width - 220);
    juce::Font font(juce::FontOptions(14.0f));
    // Use a simple approximation: average character width * text length
    // This is a rough estimate since getStringWidth is deprecated
    int estimatedWidth = static_cast<int>(message.text.length() * 8.5f); // Approximate char width
    int numLines = (estimatedWidth / maxWidth) + 1;
    return (numLines * 20) + 20; // 20px per line + padding
}

//==============================================================================
void ChatComponent::LLMThread::run()
{
    if (owner == nullptr || owner->llmService == nullptr)
        return;
    
    if (requestType == 0) // Beat generation
    {
        auto result = owner->llmService->generateBeat(prompt);
        
        juce::MessageManager::callAsync([this, result]()
        {
            if (owner == nullptr) return;
            
            owner->showLoading(false);
            
            if (result.success)
            {
                if (owner->sessionState != nullptr)
                {
                    owner->sessionState->updateFromBeatData(result.beatData);
                }
                else
                {
                    owner->addMessage("Beat generated, but session state not available.", false);
                }
            }
            else
            {
                owner->showError(result.errorMessage);
            }
        });
    }
    else if (requestType == 1) // Component adjustment
    {
        auto result = owner->llmService->adjustComponent(componentName, adjustment, currentBeat);
        
        juce::MessageManager::callAsync([this, result]()
        {
            if (owner == nullptr) return;
            
            owner->showLoading(false);
            
            if (result.success)
            {
                if (owner->sessionState != nullptr)
                {
                    owner->sessionState->updateComponent(componentName, result.componentData);
                }
                else
                {
                    owner->addMessage("Component adjusted, but session state not available.", false);
                }
            }
            else
            {
                owner->showError(result.errorMessage);
            }
        });
    }
    else if (requestType == 2) // General chat
    {
        auto result = owner->llmService->chat(prompt, owner->conversationHistory);
        
        juce::MessageManager::callAsync([this, result]()
        {
            if (owner == nullptr) return;
            
            owner->showLoading(false);
            
            if (result.success)
            {
                owner->addMessage(result.message, false);
            }
            else
            {
                owner->showError(result.errorMessage);
            }
        });
    }
}

