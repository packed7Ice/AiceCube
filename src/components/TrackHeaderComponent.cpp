#include "TrackHeaderComponent.h"

TrackHeaderComponent::TrackHeaderComponent(std::shared_ptr<Track> t, int index)
    : track(t), trackIndex(index)
{
    addAndMakeVisible(nameLabel);
    nameLabel.setText(track->name, juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    
    addAndMakeVisible(muteButton);
    muteButton.setClickingTogglesState(true);
    muteButton.setToggleState(track->mute, juce::dontSendNotification);
    muteButton.onClick = [this] { track->mute = muteButton.getToggleState(); };
    muteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    
    addAndMakeVisible(soloButton);
    soloButton.setClickingTogglesState(true);
    soloButton.setToggleState(track->solo, juce::dontSendNotification);
    soloButton.onClick = [this] { track->solo = soloButton.getToggleState(); };
    soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow);
    
    addAndMakeVisible(volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::LinearBar);
    volumeSlider.setRange(0.0, 1.0);
    volumeSlider.setValue(track->volume, juce::dontSendNotification);
    volumeSlider.onValueChange = [this] { track->volume = (float)volumeSlider.getValue(); };
}

void TrackHeaderComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::black);
    g.drawRect(getLocalBounds(), 1);
    
    // Type indicator
    juce::Colour typeColor;
    switch (track->type) {
        case TrackType::Audio: typeColor = juce::Colours::lightblue; break;
        case TrackType::Midi: typeColor = juce::Colours::orange; break;
        case TrackType::Bus: typeColor = juce::Colours::lightgreen; break;
        case TrackType::Master: typeColor = juce::Colours::red; break;
    }
    
    g.setColour(typeColor);
    g.fillRect(0, 0, 5, getHeight());
}

void TrackHeaderComponent::resized()
{
    auto area = getLocalBounds().reduced(2);
    area.removeFromLeft(5); // Type strip
    
    auto topRow = area.removeFromTop(20);
    nameLabel.setBounds(topRow);
    
    auto controls = area.removeFromBottom(20);
    muteButton.setBounds(controls.removeFromLeft(20));
    controls.removeFromLeft(2);
    soloButton.setBounds(controls.removeFromLeft(20));
    controls.removeFromLeft(2);
    volumeSlider.setBounds(controls);
}
