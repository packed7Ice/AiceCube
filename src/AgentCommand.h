#pragma once
#include <juce_core/juce_core.h>

enum class AgentAction {
    Generate,
    Replace,
    Extend,
    ClearRegion,
    Humanize
};

enum class MusicalRole {
    Melody,
    Bass,
    Drums,
    Chords,
    Pad,
    FX
};

struct AgentCommand {
    AgentAction action = AgentAction::Generate;
    MusicalRole role   = MusicalRole::Melody;

    int track      = 0; // 0-based index, or -1 if new/auto
    int barsStart  = 1;
    int barsEnd    = 4;

    juce::String style;   // "dark", "ambient", "lofi" etc.
    int density   = 3;    // 1-5
    float swing   = 0.0f; // 0.0-1.0
    
    juce::String targetTrackName; // Helper for track finding

    juce::String toString() const
    {
        juce::String str;
        str << "Action: " << (int)action << ", Role: " << (int)role 
            << ", Bars: " << barsStart << "-" << barsEnd
            << ", Style: " << style;
        return str;
    }
};
