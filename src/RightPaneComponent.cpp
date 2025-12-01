#include "RightPaneComponent.h"

RightPaneComponent::RightPaneComponent()
{
    addAndMakeVisible(agentPanel);
}

void RightPaneComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds().removeFromLeft(1), 1); // Separator
}

void RightPaneComponent::resized()
{
    auto bounds = getLocalBounds();
    agentPanel.setBounds(bounds);
}
