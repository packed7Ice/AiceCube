#include "TransportBarComponent.h"

TransportBarComponent::TransportBarComponent(AppState& state) : appState(state)
{
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(importButton);
    addAndMakeVisible(tempoLabel);
    addAndMakeVisible(tempoSlider);
    // addAndMakeVisible(positionLabel); // Removed from header in previous steps or not? Let's check header.

    playButton.onClick = [this] { if (onPlayClicked) onPlayClicked(); };
    stopButton.onClick = [this] { if (onStopClicked) onStopClicked(); };
    importButton.onClick = [this] { if (onImportAudioClicked) onImportAudioClicked(); };

    tempoLabel.setText("BPM", juce::dontSendNotification);
    
    tempoSlider.setRange(20.0, 300.0, 1.0);
    tempoSlider.setValue(appState.tempoBpm, juce::dontSendNotification);
    tempoSlider.onValueChange = [this] { appState.tempoBpm = tempoSlider.getValue(); };
}

void TransportBarComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.2f));
    g.setColour(juce::Colours::white);
    g.drawRect(getLocalBounds(), 1);
}

void TransportBarComponent::resized()
{
    auto area = getLocalBounds().reduced(5);
    
    playButton.setBounds(area.removeFromLeft(60));
    area.removeFromLeft(5);
    stopButton.setBounds(area.removeFromLeft(60));
    area.removeFromLeft(20);
    
    importButton.setBounds(area.removeFromLeft(60));
    area.removeFromLeft(20);
    
    tempoLabel.setBounds(area.removeFromLeft(40));
    tempoSlider.setBounds(area.removeFromLeft(100));
}
