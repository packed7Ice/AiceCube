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
    
    addAndMakeVisible(pluginButton);
    pluginButton.setButtonText("Inst");
    pluginButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
    pluginButton.onClick = [this] { if (onPluginButtonClicked) onPluginButtonClicked(track.get()); };
    
    if (!track->instrumentPlugin)
        pluginButton.setVisible(false);
}

void TrackHeaderComponent::mouseDown(const juce::MouseEvent& e)
{
    if (onSelect) onSelect();
    
    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu m;
        m.addItem(1, "Delete Track");
        
        m.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1)
            {
                if (onDeleteTrack) onDeleteTrack(track.get());
            }
        });
    }
}

void TrackHeaderComponent::setSelected(bool s)
{
    selected = s;
    repaint();
}

void TrackHeaderComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.1f));
    
    if (selected)
    {
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.fillAll();
        g.setColour(juce::Colours::orange);
        g.drawRect(getLocalBounds(), 2);
    }
    else
    {
        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds(), 1);
    }
    
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
    auto area = getLocalBounds().reduced(2);
    
    // Track Type Indicator
    area.removeFromLeft(4); 
    
    // Top Row: Name + Inst
    auto topRow = area.removeFromTop(20);
    pluginButton.setBounds(topRow.removeFromRight(30));
    topRow.removeFromRight(2);
    nameLabel.setBounds(topRow);
    
    area.removeFromTop(2);
    
    // Bottom Row: Buttons
    auto buttonRow = area.removeFromTop(20);
    int btnW = buttonRow.getWidth() / 4;
    
    muteButton.setBounds(buttonRow.removeFromLeft(btnW).reduced(1));
    soloButton.setBounds(buttonRow.removeFromLeft(btnW).reduced(1));
    recButton.setBounds(buttonRow.removeFromLeft(btnW).reduced(1));
    automationButton.setBounds(buttonRow.reduced(1));
}
