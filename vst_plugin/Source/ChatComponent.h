#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "OpenAIService.h"
#include "SessionState.h"

class ChatComponent : public juce::Component,
                      public juce::TextEditor::Listener,
                      public juce::Button::Listener,
                      public SessionState::Listener
{
public:
    ChatComponent();
    ~ChatComponent() override;

    void setService(OpenAIService* svc)     { service = svc; }
    void setSessionState(SessionState* state);

    void paint(juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button*) override;
    void textEditorReturnKeyPressed(juce::TextEditor&) override;
    void beatChanged() override;

    // Called when the Settings button is clicked
    std::function<void()> onSettingsClicked;

private:
    juce::TextEditor chatLog;
    juce::TextEditor inputField;
    juce::TextButton sendButton    { "Send" };
    juce::TextButton settingsButton{ "Settings" };

    OpenAIService* service      = nullptr;
    SessionState*  sessionState = nullptr;
    bool           isLoading    = false;

    juce::String logText;

    struct LLMThread;
    std::unique_ptr<LLMThread> llmThread;

    void sendMessage();
    void appendMessage(const juce::String& speaker, const juce::String& text);
    void setLoading(bool loading);
    void handleLLMResult(const OpenAIService::Response& result);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChatComponent)
};
