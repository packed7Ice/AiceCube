#include "MixerComponent.h"

MixerComponent::MixerComponent(ProjectState& state) : projectState(state)
{
}

void MixerComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.3f));
    g.setColour(juce::Colours::white);
    g.drawText("Mixer", getLocalBounds(), juce::Justification::centred, true);
}

void MixerComponent::resized()
{
}
