#include "AgentPanel.h"

AgentPanel::AgentPanel()
{
    // Log Editor setup
    logEditor.setMultiLine(true);
    logEditor.setReadOnly(true);
    logEditor.setScrollbarsShown(true);
    logEditor.setCaretVisible(false);
    logEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    logEditor.setColour(juce::TextEditor::textColourId, juce::Colours::lightgreen);
    logEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(logEditor);

    // Input Editor setup
    inputEditor.setMultiLine(false);
    inputEditor.setReturnKeyStartsNewLine(false);
    inputEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    inputEditor.setColour(juce::TextEditor::textColourId, juce::Colours::lightgreen);
    inputEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::green);
    inputEditor.setText("Type command here...");
    inputEditor.addListener(this);
    addAndMakeVisible(inputEditor);
}

void AgentPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    // Optional: Draw a border or separator
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
}

void AgentPanel::resized()
{
    auto bounds = getLocalBounds().reduced(4);
    
    // Input at the bottom, height 30
    auto inputBounds = bounds.removeFromBottom(30);
    inputEditor.setBounds(inputBounds);
    
    // Gap
    bounds.removeFromBottom(4);

    // Log takes remaining space
    logEditor.setBounds(bounds);
}

void AgentPanel::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    if (&editor == &inputEditor)
    {
        juce::String command = inputEditor.getText();
        inputEditor.clear();

        // Echo command to log
        logMessage("> " + command);

        // Callback
        if (onCommandEntered)
            onCommandEntered(command);
    }
}

void AgentPanel::logMessage(const juce::String& message)
{
    logEditor.moveCaretToEnd();
    logEditor.insertTextAtCaret(message + "\n");
}
