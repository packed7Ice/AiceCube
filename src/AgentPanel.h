#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AgentPanel : public juce::Component,
                   public juce::TextEditor::Listener
{
public:
    AgentPanel();
    ~AgentPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // TextEditor::Listener override
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

    // Log output helper
    void logMessage(const juce::String& message);

    std::function<void(const juce::String&)> onCommandEntered;

private:
    juce::TextEditor logEditor;
    juce::TextEditor inputEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AgentPanel)
};
