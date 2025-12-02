#include "ProjectState.h"

ProjectState::ProjectState()
{
}

std::shared_ptr<Track> ProjectState::addTrack(TrackType type, const juce::String& name)
{
    auto track = std::make_shared<Track>();
    track->type = type;
    track->name = name;
    tracks.push_back(track);
    return track;
}

void ProjectState::removeTrack(int index)
{
    if (index >= 0 && index < tracks.size())
    {
        tracks.erase(tracks.begin() + index);
    }
}

void ProjectState::addClip(int trackIndex, const Clip& clip)
{
    if (auto* track = getTrack(trackIndex))
    {
        track->clips.push_back(clip);
    }
}

void ProjectState::addClip(int trackIndex, double startBeat, double lengthBeats)
{
    Clip clip;
    clip.startBeat = startBeat;
    clip.lengthBeats = lengthBeats;
    clip.trackIndex = trackIndex;
    addClip(trackIndex, clip);
}
