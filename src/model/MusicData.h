#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
enum class TrackType
{
    Audio,
    Midi,
    Bus,
    Master
};

//==============================================================================
struct Note {
    int pitch;
    double startTime;
    double duration;
    int velocity;
};

struct Sequence {
    std::vector<Note> notes;
};

//==============================================================================
struct PluginSlot
{
    std::shared_ptr<juce::AudioPluginInstance> instance;
    bool bypassed = false;
    juce::String identifier;
};

//==============================================================================
struct Clip
{
    double startBeat = 0.0;
    double lengthBeats = 4.0;
    int trackIndex = 0;
    bool isMidi = true;
    
    juce::String name = "Clip";
    
    // MIDI
    juce::MidiMessageSequence midiSequence;
    
    // Audio
    juce::File audioFile;
    
    // Appearance
    juce::Colour clipColor = juce::Colours::lightblue;
    
    // Audio properties
    float gain = 1.0f;
    double fadeIn = 0.0;
    double fadeOut = 0.0;
    
    // Helper
    double getEndBeat() const { return startBeat + lengthBeats; }
};

//==============================================================================
struct AutomationPoint
{
    double time; // Beats
    float value; // 0.0 - 1.0
};

struct AutomationCurve
{
    juce::String parameterID;
    std::vector<AutomationPoint> points;
    bool active = false;
};

//==============================================================================
struct Track
{
    TrackType type = TrackType::Midi;
    juce::String name;
    
    // Clips
    std::vector<Clip> clips;
    
    // Plugins
    std::shared_ptr<PluginSlot> instrumentPlugin; // For MIDI tracks
    std::vector<std::shared_ptr<PluginSlot>> insertPlugins;
    
    // Automation
    std::vector<AutomationCurve> automationCurves;
    
    // Mixer
    float volume = 1.0f;
    float pan = 0.0f;
    bool mute = false;
    bool solo = false;
    bool arm = false;
    
    juce::Colour trackColor = juce::Colours::grey;
};
