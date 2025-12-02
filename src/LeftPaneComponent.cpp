#include "LeftPaneComponent.h"

LeftPaneComponent::LeftPaneComponent(AppState& state) 
    : appState(state), trackHeaders(state)
{
    addAndMakeVisible(trackHeaders);
}

void LeftPaneComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.1f));
}

void LeftPaneComponent::resized()
{
    trackHeaders.setBounds(getLocalBounds());
}
