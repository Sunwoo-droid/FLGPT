/*
  ==============================================================================

    ChatComponent.h
    Created: [Date]
    Author: FLGPT

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include "LLMService.h"
#include "SessionState.h"

//==============================================================================
/**
    Embedded chat UI component for the FLGPT plugin.
    Handles user input, displays chat history, and manages beat generation.
*/
class ChatComponent : public juce::Component,
                      public juce::Button::Listener,
                      public juce::TextEditor::Listener,
                      public SessionState::Listener
{
public:
    //==============================================================================
    ChatComponent();
    ~ChatComponent() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    //==============================================================================
    // Button and text editor listeners
    void buttonClicked (juce::Button* button) override;
    void textEditorReturnKeyPressed (juce::TextEditor& editor) override;
    
    //==============================================================================
    // SessionState listener
    void beatDataChanged() override;
    void componentUpdated(const juce::String& componentName) override;
    
    //==============================================================================
    // Setup
    void setLLMService(LLMService* service);
    void setSessionState(SessionState* state);
    
    //==============================================================================
    // Message structure
    struct ChatMessage
    {
        juce::String text;
        bool isUser;
        juce::Time timestamp;
    };

private:
    //==============================================================================
    // UI Components
    juce::TextEditor inputField;
    juce::TextButton sendButton;
    juce::TextButton settingsButton;
    juce::Viewport chatViewport;
    juce::Component chatContent;
    
    juce::Array<ChatMessage> messages;
    juce::Array<juce::var> conversationHistory;
    
    // References
    LLMService* llmService = nullptr;
    SessionState* sessionState = nullptr;
    
    // State
    bool isLoading = false;
    
    // Background thread for LLM calls
    class LLMThread : public juce::Thread
    {
    public:
        LLMThread(ChatComponent* owner) : juce::Thread("LLMThread"), owner(owner) {}
        void run() override;
        
        juce::String prompt;
        juce::String componentName;
        juce::String adjustment;
        juce::var currentBeat;
        int requestType; // 0=beat, 1=component, 2=chat
        
    private:
        ChatComponent* owner;
    };
    
    std::unique_ptr<LLMThread> llmThread;
    
    //==============================================================================
    // Methods
    void sendMessage();
    void addMessage(const juce::String& text, bool isUser);
    void displayBeatInfo(const SessionState::BeatData& beat);
    void updateChatDisplay();
    
    // LLM interaction
    void handleBeatGeneration(const juce::String& prompt);
    void handleComponentAdjustment(const juce::String& componentName, const juce::String& adjustment);
    void handleGeneralChat(const juce::String& message);
    
    // Helper methods
    bool isBeatRequest(const juce::String& prompt);
    void showError(const juce::String& errorMessage);
    void showLoading(bool show);
    
    // UI helpers
    void drawMessage(juce::Graphics& g, const ChatMessage& message, int y, int width);
    int calculateMessageHeight(const ChatMessage& message, int width);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChatComponent)
};

