#include "ProjectState.h"

ProjectState::ProjectState()
{
    // Default tracks removed as per user request
}

std::shared_ptr<Track> ProjectState::addTrack(TrackType type, const juce::String& name)
{
    auto newTrack = std::make_shared<Track>();
    newTrack->id = juce::Uuid();
    newTrack->type = type;
    newTrack->name = name;
    
    // Random color
    juce::Random r;
    newTrack->trackColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
    
    tracks.push_back(newTrack);
    return newTrack;
}

void ProjectState::addClip(int trackIndex, const Clip& clip)
{
    if (auto* t = getTrack(trackIndex))
    {
        t->clips.push_back(clip);
    }
}

void ProjectState::addClip(int trackIndex, double startBeat, double lengthBeats)
{
    if (auto* t = getTrack(trackIndex))
    {
        Clip newClip;
        newClip.startBeat = startBeat;
        newClip.lengthBeats = lengthBeats;
        newClip.trackIndex = trackIndex;
        
        if (t->type == TrackType::Midi)
        {
            newClip.isMidi = true;
            newClip.name = "Midi Clip";
            t->clips.push_back(newClip);
        }
        // Audio clips require file selection, skipping for empty creation
    }
}
