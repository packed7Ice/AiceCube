
#include "TransportBarComponent.h"

TransportBarComponent::TransportBarComponent(ProjectState& state) : projectState(state)
{
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(recordButton);
    addAndMakeVisible(importButton);
    addAndMakeVisible(saveButton); // Added
    addAndMakeVisible(loadButton); // Added
    addAndMakeVisible(tempoLabel);
    addAndMakeVisible(tempoSlider);

    playButton.onClick = [this] { if (onPlayClicked) onPlayClicked(); };
    stopButton.onClick = [this] { if (onStopClicked) onStopClicked(); };
    recordButton.onClick = [this] { if (onRecordClicked) onRecordClicked(); };
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    
    importButton.onClick = [this] { if (onImportAudioClicked) onImportAudioClicked(); };
    
    saveButton.onClick = [this] { if (onSaveClicked) onSaveClicked(); }; // Added
    
    loadButton.onClick = [this] { if (onLoadClicked) onLoadClicked(); }; // Added
    
    tempoLabel.setText("Tempo:", juce::dontSendNotification); // Modified
    
    tempoSlider.setRange(20.0, 300.0, 1.0);
    tempoSlider.setValue(projectState.tempo, juce::dontSendNotification);
    tempoSlider.onValueChange = [this] { projectState.tempo = tempoSlider.getValue(); };
}

void TransportBarComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey); // Modified
}

void TransportBarComponent::resized()
{
    auto area = getLocalBounds().reduced(4); // Modified
    
    playButton.setBounds(area.removeFromLeft(60).reduced(2)); // Modified
    area.removeFromLeft(5);
    stopButton.setBounds(area.removeFromLeft(60).reduced(2)); // Modified
    area.removeFromLeft(5);
    recordButton.setBounds(area.removeFromLeft(60).reduced(2));
    area.removeFromLeft(20);
    
    importButton.setBounds(area.removeFromLeft(80).reduced(2)); // Modified
    saveButton.setBounds(area.removeFromLeft(60).reduced(2)); // Added
    loadButton.setBounds(area.removeFromLeft(60).reduced(2)); // Added
    
    area.removeFromLeft(20); // Added
    
    tempoLabel.setBounds(area.removeFromLeft(60)); // Modified
    tempoSlider.setBounds(area.removeFromLeft(100));
}
