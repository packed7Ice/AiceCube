#pragma once
#include <juce_core/juce_core.h>
#include "MusicData.h"

struct Clip
{
    double startBeat = 0.0;
    double lengthBeats = 4.0;
    int trackIndex = 0;
    juce::Array<Note> notes;
    
    // Helper to get end beat
    double getEndBeat() const { return startBeat + lengthBeats; }
};

struct Track
{
    juce::String name;
    juce::Array<Clip> clips;
    
    bool isMuted = false;
    bool isSoloed = false;
    float volume = 1.0f; // 0.0 to 1.0
};

class AppState
{
public:
    AppState();
    ~AppState() = default;

    // Transport
    double tempoBpm = 120.0;
    double playheadBeat = 0.0;
    bool isPlaying = false;

    // Data
    juce::Array<Track> tracks;

    // Methods
    void updatePlayhead(double deltaBeats);
    void addTrack(const juce::String& name);
    void addClip(int trackIndex, const Clip& clip);
    
    // Listeners could be added here later (ChangeBroadcaster)
};
