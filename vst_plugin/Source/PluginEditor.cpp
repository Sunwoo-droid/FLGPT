#include "PluginProcessor.h"
#include "PluginEditor.h"

FLGPTPluginAudioProcessorEditor::FLGPTPluginAudioProcessorEditor(FLGPTPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(700, 500);
    setResizable(true, true);
    setResizeLimits(400, 300, 1200, 900);

    chatComponent.setService(&audioProcessor.getOpenAIService());
    chatComponent.setSessionState(&audioProcessor.getSessionState());
    chatComponent.onSettingsClicked = [this] { showSettings(); };
    addAndMakeVisible(chatComponent);

    loadAndApplyApiKey();
}

FLGPTPluginAudioProcessorEditor::~FLGPTPluginAudioProcessorEditor() {}

void FLGPTPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    // Header bar
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(0, 0, getWidth(), 40);

    g.setColour(juce::Colour(0xff4a9eff));
    g.setFont(juce::Font(juce::FontOptions(18.0f, juce::Font::bold)));
    g.drawText("FLGPT", 14, 8, 80, 24, juce::Justification::left);

    g.setColour(juce::Colour(0xff666666));
    g.setFont(juce::Font(juce::FontOptions(12.0f)));
    g.drawText("AI Beat Creator for FL Studio", 96, 12, 300, 18, juce::Justification::left);
}

void FLGPTPluginAudioProcessorEditor::resized()
{
    auto b = getLocalBounds();
    b.removeFromTop(40);
    chatComponent.setBounds(b);
}

void FLGPTPluginAudioProcessorEditor::showSettings()
{
    auto* dialog = new juce::AlertWindow("Settings",
                                          "Enter your OpenAI API key:",
                                          juce::MessageBoxIconType::NoIcon);
    dialog->addTextEditor("apiKey", audioProcessor.getOpenAIService().getApiKey(), "API Key:");
    dialog->addButton("Save",   1);
    dialog->addButton("Cancel", 0);

    dialog->enterModalState(true,
        juce::ModalCallbackFunction::create([this, dialog](int result)
        {
            if (result == 1)
            {
                const juce::String key = dialog->getTextEditorContents("apiKey").trim();
                audioProcessor.getOpenAIService().setApiKey(key);
                saveApiKey(key);
            }
        }),
        true /* delete dialog on close */);
}

void FLGPTPluginAudioProcessorEditor::loadAndApplyApiKey()
{
    const juce::String key = loadApiKey();
    if (key.isNotEmpty())
        audioProcessor.getOpenAIService().setApiKey(key);
}

juce::File FLGPTPluginAudioProcessorEditor::getSettingsFile()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile("FLGPT")
               .getChildFile("apikey.txt");
}

void FLGPTPluginAudioProcessorEditor::saveApiKey(const juce::String& key)
{
    auto f = getSettingsFile();
    f.getParentDirectory().createDirectory();
    f.replaceWithText(key);
}

juce::String FLGPTPluginAudioProcessorEditor::loadApiKey()
{
    const auto f = getSettingsFile();
    return f.existsAsFile() ? f.loadFileAsString().trim() : juce::String{};
}
