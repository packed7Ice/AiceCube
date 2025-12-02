#include "TrackHeaderListComponent.h"

TrackHeaderListComponent::TrackHeaderListComponent(ProjectState& state, AudioEngine& engine) 
    : projectState(state), audioEngine(engine)
{
    addAndMakeVisible(addTrackButton);
    addTrackButton.onClick = [this] {
        juce::PopupMenu m;
        m.addItem(1, "Add Audio Track");
        m.addItem(2, "Add MIDI Track");
        
        juce::PopupMenu instMenu;
        int id = 100;
        std::vector<juce::PluginDescription> instruments;
        
        for (const auto& desc : audioEngine.getKnownPluginList().getTypes())
        {
            if (desc.isInstrument)
            {
                instMenu.addItem(id, desc.name);
                instruments.push_back(desc);
                id++;
            }
        }
        m.addSubMenu("Add Instrument Track", instMenu);
        
        m.showMenuAsync(juce::PopupMenu::Options(), [this, instruments](int result) {
            if (result == 1) projectState.addTrack(TrackType::Audio, "Audio Track");
            else if (result == 2) projectState.addTrack(TrackType::Midi, "Midi Track");
            else if (result >= 100 && result < 100 + instruments.size())
            {
                auto& desc = instruments[result - 100];
                auto track = projectState.addTrack(TrackType::Midi, desc.name);
                audioEngine.setInstrumentPlugin(track.get(), desc);
            }
            
            if (result != 0) 
            {
                updateTrackList();
                if (onTrackListChanged) onTrackListChanged();
            }
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
    int trackHeight = 100;
    
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
        header->onPluginButtonClicked = [this](Track* t) {
            audioEngine.togglePluginWindow(t);
        };
        header->onDeleteTrack = [this](Track* t) {
            // Find index
            int index = -1;
            for (int j = 0; j < projectState.tracks.size(); ++j)
            {
                if (projectState.tracks[j].get() == t)
                {
                    index = j;
                    break;
                }
            }
            
            if (index != -1)
            {
                projectState.removeTrack(index);
                audioEngine.updateGraph();
                updateTrackList();
                if (onTrackListChanged) onTrackListChanged();
            }
        };
        headers.add(header);
        addAndMakeVisible(header);
    }
    
    resized();
}
