#include "TransportBarComponent.h"

TransportBarComponent::TransportBarComponent(AppState& state) : appState(state)
{
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(tempoLabel);
    addAndMakeVisible(positionLabel);

    playButton.onClick = [this] { if (onPlayClicked) onPlayClicked(); };
    stopButton.onClick = [this] { if (onStopClicked) onStopClicked(); };

    tempoLabel.setText(juce::String(appState.tempoBpm), juce::dontSendNotification);
    tempoLabel.setEditable(true);
    tempoLabel.onTextChange = [this] {
        double newBpm = tempoLabel.getText().getDoubleValue();
        if (newBpm > 10.0 && newBpm < 300.0)
        {
            appState.tempoBpm = newBpm;
        }
    };
    
    positionLabel.setText("Bar: 1.0", juce::dontSendNotification); // Placeholder
}

void TransportBarComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.2f));
    g.setColour(juce::Colours::white);
    g.drawRect(getLocalBounds(), 1);
}

void TransportBarComponent::resized()
{
    auto bounds = getLocalBounds().reduced(4);
    playButton.setBounds(bounds.removeFromLeft(60));
    bounds.removeFromLeft(4);
    stopButton.setBounds(bounds.removeFromLeft(60));
    bounds.removeFromLeft(10);
    tempoLabel.setBounds(bounds.removeFromLeft(100));
    positionLabel.setBounds(bounds.removeFromLeft(100));
}
