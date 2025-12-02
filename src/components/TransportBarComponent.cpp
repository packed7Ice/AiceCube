#include "TransportBarComponent.h"

TransportBarComponent::TransportBarComponent(ProjectState& state) : projectState(state)
{
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(recordButton);
    addAndMakeVisible(importButton);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(settingsButton);
    addAndMakeVisible(filesButton);
    addAndMakeVisible(tempoLabel);
    addAndMakeVisible(tempoSlider);

    playButton.onClick = [this] { if (onPlayClicked) onPlayClicked(); };
    stopButton.onClick = [this] { if (onStopClicked) onStopClicked(); };
    recordButton.onClick = [this] { if (onRecordClicked) onRecordClicked(); };
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    
    importButton.onClick = [this] { if (onImportAudioClicked) onImportAudioClicked(); };
    
    saveButton.onClick = [this] { if (onSaveClicked) onSaveClicked(); };
    loadButton.onClick = [this] { if (onLoadClicked) onLoadClicked(); };
    loadButton.onClick = [this] { if (onLoadClicked) onLoadClicked(); };
    settingsButton.onClick = [this] { if (onSettingsClicked) onSettingsClicked(); };
    
    filesButton.setClickingTogglesState(true);
    filesButton.setToggleState(true, juce::dontSendNotification); // Default visible
    filesButton.onClick = [this] { if (onFilesClicked) onFilesClicked(); };
    
    addAndMakeVisible(metronomeButton);
    metronomeButton.setClickingTogglesState(true);
    metronomeButton.setToggleState(projectState.metronomeEnabled, juce::dontSendNotification);
    metronomeButton.onClick = [this] { projectState.metronomeEnabled = metronomeButton.getToggleState(); };
    
    addAndMakeVisible(loopButton);
    loopButton.setClickingTogglesState(true);
    loopButton.setToggleState(projectState.isLooping, juce::dontSendNotification);
    loopButton.onClick = [this] { projectState.isLooping = loopButton.getToggleState(); };
    
    tempoLabel.setText("Tempo:", juce::dontSendNotification);
    
    tempoSlider.setRange(20.0, 300.0, 1.0);
    tempoSlider.setValue(projectState.tempo, juce::dontSendNotification);
    tempoSlider.onValueChange = [this] { projectState.tempo = tempoSlider.getValue(); };
}

void TransportBarComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void TransportBarComponent::resized()
{
    auto area = getLocalBounds().reduced(4);
    
    playButton.setBounds(area.removeFromLeft(60).reduced(2));
    area.removeFromLeft(5);
    stopButton.setBounds(area.removeFromLeft(60).reduced(2));
    area.removeFromLeft(5);
    recordButton.setBounds(area.removeFromLeft(60).reduced(2));
    area.removeFromLeft(20);
    
    importButton.setBounds(area.removeFromLeft(80).reduced(2));
    area.removeFromLeft(5);
    saveButton.setBounds(area.removeFromLeft(60).reduced(2));
    area.removeFromLeft(5);
    loadButton.setBounds(area.removeFromLeft(60).reduced(2));
    area.removeFromLeft(5);
    settingsButton.setBounds(area.removeFromLeft(80).reduced(2));
    settingsButton.setBounds(area.removeFromLeft(80).reduced(2));
    area.removeFromLeft(5);
    filesButton.setBounds(area.removeFromLeft(60).reduced(2));
    area.removeFromLeft(5);
    metronomeButton.setBounds(area.removeFromLeft(80).reduced(2));
    area.removeFromLeft(5);
    loopButton.setBounds(area.removeFromLeft(60).reduced(2));
    
    area.removeFromLeft(20);
    tempoLabel.setBounds(area.removeFromLeft(50));
    tempoSlider.setBounds(area.removeFromLeft(150));
}
