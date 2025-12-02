#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h> // For AudioProcessorGraph
#include "MusicData.h"

enum class TrackType
{
    Audio,
    Midi,
    Bus
};

enum class ClipType
{
    Audio,
    Midi
};

struct Clip
{
    juce::String name = "Clip";
    ClipType type = ClipType::Midi;

    double startBeat = 0.0;
    double lengthBeats = 4.0;
    int trackIndex = 0;
    
    // Content
    juce::Array<Note> notes;                  // For MIDI (Simple representation)
    juce::MidiMessageSequence midiSequence;   // For MIDI (Full representation)
    juce::File audioFile;                     // For Audio
    
    // Trimming / Offset
    double sourceStartBeat = 0.0; // Offset into the source (audio file or midi pattern)
    
    // Helper to get end beat
    double getEndBeat() const { return startBeat + lengthBeats; }
};

struct Track
{
    juce::String name;
    TrackType type = TrackType::Midi;
    juce::Array<Clip> clips;
    
    bool isMuted = false;
    bool isSoloed = false;
    float volume = 1.0f; // 0.0 to 1.0
    float pan = 0.0f;    // -1.0 to 1.0

    // Plugin Chain
    // We will store the NodeID or Ptr to the AudioProcessorGraph::Node
    // For now, let's keep it simple. The AudioEngine will likely manage the Graph.
    // But we need to know which node corresponds to this track.
    juce::AudioProcessorGraph::Node::Ptr processorNode;
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
    void addTrack(const juce::String& name, TrackType type = TrackType::Midi);
    void addClip(int trackIndex, const Clip& clip);
    
    // Listeners could be added here later (ChangeBroadcaster)
};
