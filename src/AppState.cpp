#include "AppState.h"

AppState::AppState()
{
    // Add some default tracks
    addTrack("Melody");
    addTrack("Drums");
    addTrack("Bass");
}

void AppState::updatePlayhead(double deltaBeats)
{
    if (isPlaying)
    {
        playheadBeat += deltaBeats;
        // Loop logic could go here
    }
}

void AppState::addTrack(const juce::String& name)
{
    Track t;
    t.name = name;
    tracks.add(t);
}

void AppState::addClip(int trackIndex, const Clip& clip)
{
    if (trackIndex >= 0 && trackIndex < tracks.size())
    {
        tracks.getReference(trackIndex).clips.add(clip);
    }
}
