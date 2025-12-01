#pragma once
#include <juce_core/juce_core.h>

struct AgentCommand
{
    enum class Type { AddMelody, AddDrums, ModifyMix, Unknown };

    Type type = Type::Unknown;
    int track = 0;
    int barsStart = 0;
    int barsEnd = 0;

    juce::String mood;      // "dark", "bright", "tense" etc.
    float intensity = 0.0f; // 0.0 to 1.0
    juce::String extra;     // Style or extra parameters
    juce::String targetTrackName; // Optional: inferred from command
    
    juce::String toString() const
    {
        juce::String typeStr;
        switch (type)
        {
            case Type::AddMelody: typeStr = "AddMelody"; break;
            case Type::AddDrums:  typeStr = "AddDrums"; break;
            case Type::ModifyMix: typeStr = "ModifyMix"; break;
            default:              typeStr = "Unknown"; break;
        }

        return "Type: " + typeStr + 
               ", Track: " + juce::String(track) +
               ", Bars: " + juce::String(barsStart) + "-" + juce::String(barsEnd) +
               ", Mood: " + mood +
               ", Intensity: " + juce::String(intensity, 2);
    }
};
