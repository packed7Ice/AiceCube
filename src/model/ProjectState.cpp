#include "ProjectState.h"

ProjectState::ProjectState()
{
    // Default tracks
    addTrack(TrackType::Midi, "Melody");
    addTrack(TrackType::Midi, "Chords");
    addTrack(TrackType::Audio, "Audio");
}

void ProjectState::addTrack(TrackType type, const juce::String& name)
{
    auto newTrack = std::make_shared<Track>();
    newTrack->type = type;
    newTrack->name = name;
    
    // Random color
    juce::Random r;
    newTrack->trackColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
    
    tracks.push_back(newTrack);
}

void ProjectState::addClip(int trackIndex, const Clip& clip)
{
    if (auto* t = getTrack(trackIndex))
    {
        t->clips.push_back(clip);
    }
}
