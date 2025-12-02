#include "TrackHeaderComponent.h"

TrackHeaderComponent::TrackHeaderComponent(std::shared_ptr<Track> t) : track(t)
{
    addAndMakeVisible(nameLabel);
    nameLabel.setText(track->name, juce::dontSendNotification);
    nameLabel.setEditable(true);
    nameLabel.onTextChange = [this] { track->name = nameLabel.getText(); };
    
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
    
    addAndMakeVisible(recButton);
    recButton.setClickingTogglesState(true);
    recButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red.darker());
    
    addAndMakeVisible(automationButton);
    automationButton.setClickingTogglesState(true);
    automationButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::cyan);
    automationButton.onClick = [this] {
        // Toggle Automation
        bool active = automationButton.getToggleState();
        
        // Find or create Volume curve
        bool found = false;
        for (auto& curve : track->automationCurves)
        {
            if (curve.parameterID == "Volume")
            {
                curve.active = active;
                found = true;
                break;
            }
        }
        
        if (!found && active)
        {
            AutomationCurve curve;
            curve.parameterID = "Volume";
            curve.active = true;
            // Add dummy points
            curve.points.push_back({ 0.0, 0.8f });
            curve.points.push_back({ 4.0, 0.2f });
            curve.points.push_back({ 8.0, 0.8f });
            track->automationCurves.push_back(curve);
        }
        
        // Trigger repaint of timeline (via parent or message)
        // Ideally we should use a listener, but for now we rely on periodic updates or manual refresh
        // MainComponent updates timeline periodically during playback, but we might want immediate feedback.
        // We can't easily access TimelineComponent here.
    };
    
    addAndMakeVisible(volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::LinearBar);
    volumeSlider.setRange(0.0, 1.0);
    volumeSlider.setValue(track->volume, juce::dontSendNotification);
    volumeSlider.onValueChange = [this] { track->volume = (float)volumeSlider.getValue(); };
    
    addAndMakeVisible(panSlider);
    panSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    panSlider.setRange(-1.0, 1.0);
    panSlider.setValue(track->pan, juce::dontSendNotification);
    panSlider.setValue(track->pan, juce::dontSendNotification);
    panSlider.onValueChange = [this] { track->pan = (float)panSlider.getValue(); };
    
    addAndMakeVisible(pluginButton);
    pluginButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
    pluginButton.onClick = [this] { if (onPluginButtonClicked) onPluginButtonClicked(track.get()); };
    
    // Only show if plugin exists
    if (track->instrumentPlugin && track->instrumentPlugin->identifier.isNotEmpty())
        pluginButton.setVisible(true);
    else
        pluginButton.setVisible(false);
}

void TrackHeaderComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.1f));
    g.setColour(juce::Colours::black);
    g.drawRect(getLocalBounds(), 1);
    
    // Track Type Indicator
    juce::Colour typeColor;
    switch (track->type) {
        case TrackType::Audio: typeColor = juce::Colours::green; break;
        case TrackType::Midi: typeColor = juce::Colours::cyan; break;
        case TrackType::Bus: typeColor = juce::Colours::orange; break;
        default: typeColor = juce::Colours::grey; break;
    }
    g.setColour(typeColor);
    g.fillRect(0, 0, 5, getHeight());
}

void TrackHeaderComponent::resized()
{
    auto area = getLocalBounds().reduced(4);
    area.removeFromLeft(6); // Space for indicator
    
    auto topRow = area.removeFromTop(20);
    nameLabel.setBounds(topRow);
    
    area.removeFromTop(4);
    
    auto buttonRow = area.removeFromTop(20);
    int btnW = buttonRow.getWidth() / 4;
    muteButton.setBounds(buttonRow.removeFromLeft(btnW).reduced(1));
    soloButton.setBounds(buttonRow.removeFromLeft(btnW).reduced(1));
    recButton.setBounds(buttonRow.removeFromLeft(btnW).reduced(1));
    automationButton.setBounds(buttonRow.reduced(1));
    
    area.removeFromTop(4);
    
    auto volRow = area.removeFromTop(15);
    volumeSlider.setBounds(volRow);
    
    area.removeFromTop(4);
    
    panSlider.setBounds(area.removeFromLeft(30));
    
    if (pluginButton.isVisible())
    {
        area.removeFromLeft(4);
        pluginButton.setBounds(area.removeFromLeft(40));
    }
}
