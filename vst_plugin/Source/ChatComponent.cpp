#include "ChatComponent.h"

// Background thread that calls OpenAI and then posts the result back to the message thread.
// Uses SafePointer so that if ChatComponent is destroyed while the thread is running,
// the async callback safely no-ops instead of accessing freed memory.
struct ChatComponent::LLMThread : public juce::Thread
{
    LLMThread(ChatComponent* owner,
              const juce::String& msg,
              const SessionState::Beat& beat)
        : juce::Thread("FLGPT-LLM"), owner(owner), userMessage(msg), currentBeat(beat) {}

    void run() override
    {
        if (!owner || !owner->service) return;

        auto result = owner->service->sendMessage(userMessage, currentBeat);

        juce::Component::SafePointer<ChatComponent> safeOwner(owner);
        juce::MessageManager::callAsync([safeOwner, result]()
        {
            if (safeOwner == nullptr) return;
            safeOwner->handleLLMResult(result);
        });
    }

    ChatComponent*       owner;
    juce::String         userMessage;
    SessionState::Beat   currentBeat;
};

//==============================================================================
ChatComponent::ChatComponent()
{
    // Chat log: read-only TextEditor handles display + scrolling
    chatLog.setMultiLine(true);
    chatLog.setReadOnly(true);
    chatLog.setScrollbarsShown(true);
    chatLog.setFont(juce::Font(juce::FontOptions(13.0f)));
    chatLog.setColour(juce::TextEditor::backgroundColourId,      juce::Colour(0xff1e1e1e));
    chatLog.setColour(juce::TextEditor::textColourId,            juce::Colour(0xffd4d4d4));
    chatLog.setColour(juce::TextEditor::outlineColourId,         juce::Colour(0xff333333));
    chatLog.setColour(juce::TextEditor::focusedOutlineColourId,  juce::Colour(0xff333333));
    addAndMakeVisible(chatLog);

    // Input field
    inputField.setMultiLine(false);
    inputField.setReturnKeyStartsNewLine(false);
    inputField.setFont(juce::Font(juce::FontOptions(13.0f)));
    inputField.setColour(juce::TextEditor::backgroundColourId,     juce::Colour(0xff2d2d2d));
    inputField.setColour(juce::TextEditor::textColourId,           juce::Colours::white);
    inputField.setColour(juce::TextEditor::outlineColourId,        juce::Colour(0xff555555));
    inputField.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff4a9eff));
    inputField.addListener(this);
    addAndMakeVisible(inputField);

    sendButton.setColour(juce::TextButton::buttonColourId,  juce::Colour(0xff4a9eff));
    sendButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    sendButton.addListener(this);
    addAndMakeVisible(sendButton);

    settingsButton.setColour(juce::TextButton::buttonColourId,  juce::Colour(0xff3a3a3a));
    settingsButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffaaaaaa));
    settingsButton.addListener(this);
    addAndMakeVisible(settingsButton);

    appendMessage("FLGPT", "Welcome! Describe the beat you want.\n"
                            "Example: \"Create a chill lo-fi beat at 85 BPM\"\n"
                            "Or after a beat is loaded: \"Make the hi-hats more aggressive\"");
}

ChatComponent::~ChatComponent()
{
    if (sessionState) sessionState->removeListener(this);
    if (llmThread) llmThread->stopThread(3000);
}

void ChatComponent::setSessionState(SessionState* state)
{
    if (sessionState) sessionState->removeListener(this);
    sessionState = state;
    if (sessionState) sessionState->addListener(this);
}

//==============================================================================
void ChatComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void ChatComponent::resized()
{
    auto b = getLocalBounds().reduced(8);

    // Settings button top-right
    auto topRow = b.removeFromTop(30);
    settingsButton.setBounds(topRow.removeFromRight(90).reduced(0, 2));
    b.removeFromTop(6);

    // Input row at bottom
    auto inputRow = b.removeFromBottom(34);
    sendButton.setBounds(inputRow.removeFromRight(70));
    inputRow.removeFromRight(6);
    inputField.setBounds(inputRow);
    b.removeFromBottom(6);

    chatLog.setBounds(b);
}

void ChatComponent::buttonClicked(juce::Button* btn)
{
    if (btn == &sendButton)
        sendMessage();
    else if (btn == &settingsButton && onSettingsClicked)
        onSettingsClicked();
}

void ChatComponent::textEditorReturnKeyPressed(juce::TextEditor& ed)
{
    if (&ed == &inputField) sendMessage();
}

void ChatComponent::beatChanged()
{
    if (!sessionState) return;
    const auto& beat = sessionState->getBeat();

    juce::String info = "Beat ready: " + juce::String(beat.tempo) + " BPM";
    if (beat.key.isNotEmpty())   info += ", " + beat.key;
    if (beat.style.isNotEmpty()) info += " — " + beat.style;
    info += ". Tracks: ";
    for (int i = 0; i < beat.instruments.size(); ++i)
    {
        if (i > 0) info += ", ";
        info += beat.instruments[i].name;
    }
    info += ". Press Play in FL Studio to hear it.";
    appendMessage("FLGPT", info);
}

//==============================================================================
void ChatComponent::sendMessage()
{
    const juce::String text = inputField.getText().trim();
    if (text.isEmpty() || isLoading) return;

    inputField.setText({});
    appendMessage("You", text);

    if (!service || !service->hasApiKey())
    {
        appendMessage("FLGPT", "Please add your OpenAI API key in Settings first.");
        return;
    }

    SessionState::Beat currentBeat;
    if (sessionState) currentBeat = sessionState->getBeat();

    setLoading(true);
    llmThread = std::make_unique<LLMThread>(this, text, currentBeat);
    llmThread->startThread();
}

void ChatComponent::handleLLMResult(const OpenAIService::Response& result)
{
    setLoading(false);

    if (!result.success)
    {
        appendMessage("FLGPT", "Error: " + result.error);
        return;
    }

    if (result.message.isNotEmpty())
        appendMessage("FLGPT", result.message);

    if (result.action == "generate_beat" && sessionState)
        sessionState->updateBeat(result.beat);
    else if (result.action == "update_component" && sessionState)
        sessionState->updateComponent(result.componentName, result.component);
}

//==============================================================================
void ChatComponent::appendMessage(const juce::String& speaker, const juce::String& text)
{
    logText += speaker + ": " + text + "\n\n";
    chatLog.setText(logText, false);
    chatLog.moveCaretToEnd();
}

void ChatComponent::setLoading(bool loading)
{
    isLoading = loading;
    sendButton.setEnabled(!loading);
    inputField.setEnabled(!loading);
    if (loading) appendMessage("FLGPT", "...");
}
