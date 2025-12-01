#include "BottomPaneComponent.h"

BottomPaneComponent::BottomPaneComponent()
{
    addAndMakeVisible(agentPanel);
    addAndMakeVisible(editorPlaceholder);
}

void BottomPaneComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void BottomPaneComponent::resized()
{
    auto bounds = getLocalBounds();
    // Split 50/50 for now
    agentPanel.setBounds(bounds.removeFromLeft(getWidth() / 2));
    editorPlaceholder.setBounds(bounds);
}
