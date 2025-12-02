#pragma once
#include <juce_core/juce_core.h>
#include "MusicData.h"

class ProjectState
{
public:
    ProjectState();
    ~ProjectState() = default;

    // Transport
    double tempo = 120.0;
    int timeSignatureNumerator = 4;
    int timeSignatureDenominator = 4;
    
    double playheadBeat = 0.0;
    bool isPlaying = false;
    bool isLooping = false;
    double loopStart = 0.0;

    double loopEnd = 4.0;
    bool metronomeEnabled = false;

    // Data
    // Using shared_ptr for Tracks to allow easy management
    std::vector<std::shared_ptr<Track>> tracks;

    // Serialization
    juce::ValueTree serializationRoot { "Project" };

    // Methods
    std::shared_ptr<Track> addTrack(TrackType type, const juce::String& name);
    void removeTrack(int index);
    void addClip(int trackIndex, const Clip& clip);
    void addClip(int trackIndex, double startBeat, double lengthBeats);
    
    // Helpers
    Track* getTrack(int index) {
        if (index >= 0 && index < tracks.size()) return tracks[index].get();
        return nullptr;
    }
};
