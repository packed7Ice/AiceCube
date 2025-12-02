#include "TrackHeaderListComponent.h"

//==============================================================================
TrackHeaderComponent::TrackHeaderComponent(Track& t, int index)
    : track(t), trackIndex(index)
{
    addAndMakeVisible(nameLabel);
    nameLabel.setText(track.name, juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    
    addAndMakeVisible(muteButton);
    muteButton.setToggleState(track.isMuted, juce::dontSendNotification);
    muteButton.onClick = [this] { track.isMuted = muteButton.getToggleState(); };
    
    addAndMakeVisible(soloButton);
    soloButton.setToggleState(track.isSoloed, juce::dontSendNotification);
    soloButton.onClick = [this] { track.isSoloed = soloButton.getToggleState(); };
    
    addAndMakeVisible(volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::LinearBar);
    volumeSlider.setRange(0.0, 1.0);
    volumeSlider.setValue(track.volume, juce::dontSendNotification);
    volumeSlider.onValueChange = [this] { track.volume = (float)volumeSlider.getValue(); };
}

void TrackHeaderComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.2f));
    g.setColour(juce::Colours::black);
    g.drawRect(getLocalBounds(), 1);
}

void TrackHeaderComponent::resized()
{
    auto area = getLocalBounds().reduced(2);
    
    auto topRow = area.removeFromTop(20);
    nameLabel.setBounds(topRow);
    
    auto controlsRow = area.removeFromTop(20);
    muteButton.setBounds(controlsRow.removeFromLeft(20));
    soloButton.setBounds(controlsRow.removeFromLeft(20));
    volumeSlider.setBounds(controlsRow);
}

//==============================================================================
TrackHeaderListComponent::TrackHeaderListComponent(AppState& state)
    : appState(state)
{
    addAndMakeVisible(addTrackButton);
    addTrackButton.onClick = [this] {
        appState.addTrack("New Track", TrackType::Midi);
        updateTrackList();
    };
    
    updateTrackList();
}

void TrackHeaderListComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void TrackHeaderListComponent::resized()
{
    auto area = getLocalBounds();
    
    // Header for "Tracks" or Add button
    auto topBar = area.removeFromTop(30);
    addTrackButton.setBounds(topBar.removeFromRight(30).reduced(2));
    
    int headerHeight = 50;
    
    for (auto* header : headers)
    {
        header->setBounds(area.removeFromTop(headerHeight));
        area.removeFromTop(1); // Gap
    }
}

void TrackHeaderListComponent::updateTrackList()
{
    headers.clear();
    
    for (int i = 0; i < appState.tracks.size(); ++i)
    {
        auto* header = new TrackHeaderComponent(appState.tracks.getReference(i), i);
        headers.add(header);
        addAndMakeVisible(header);
    }
    
    resized();
}
