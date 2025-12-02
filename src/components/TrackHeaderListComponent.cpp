#include "TrackHeaderListComponent.h"

TrackHeaderListComponent::TrackHeaderListComponent(ProjectState& state) : projectState(state)
{
    addAndMakeVisible(addTrackButton);
    addTrackButton.onClick = [this] {
        juce::PopupMenu m;
        m.addItem(1, "Add Audio Track");
        m.addItem(2, "Add MIDI Track");
        
        m.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1) projectState.addTrack(TrackType::Audio, "Audio Track");
            else if (result == 2) projectState.addTrack(TrackType::Midi, "Midi Track");
            
            if (result != 0) updateTrackList();
        });
    };
    
    updateTrackList();
}

void TrackHeaderListComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void TrackHeaderListComponent::resized()
{
    int y = 0;
    int trackHeight = 60;
    
    for (auto* header : headers)
    {
        header->setBounds(0, y, getWidth(), trackHeight);
        y += trackHeight;
    }
    
    addTrackButton.setBounds(0, y, getWidth(), 30);
}

void TrackHeaderListComponent::updateTrackList()
{
    headers.clear();
    
    for (int i = 0; i < projectState.tracks.size(); ++i)
    {
        auto* header = new TrackHeaderComponent(projectState.tracks[i]);
        headers.add(header);
        addAndMakeVisible(header);
    }
    
    resized();
}
