#include "AgentLogic.h"

juce::Array<AgentCommand> AgentLogic::interpretInstruction(const juce::String& inputText)
{
    juce::Array<AgentCommand> commands;
    AgentCommand cmd;
    
    juce::String lowerInput = inputText.toLowerCase();

    // 1. Detect Type
    if (lowerInput.contains("melody") || lowerInput.contains("メロディ"))
    {
        cmd.type = AgentCommand::Type::AddMelody;
    }
    else if (lowerInput.contains("drum") || lowerInput.contains("beat") || lowerInput.contains("ドラム"))
    {
        cmd.type = AgentCommand::Type::AddDrums;
    }
    else if (lowerInput.contains("mix") || lowerInput.contains("volume") || lowerInput.contains("ミックス"))
    {
        cmd.type = AgentCommand::Type::ModifyMix;
    }
    else
    {
        // Default fallback for now
        cmd.type = AgentCommand::Type::Unknown;
    }

    // 2. Detect Mood
    if (lowerInput.contains("dark") || lowerInput.contains("暗い") || lowerInput.contains("sad"))
    {
        cmd.mood = "dark";
    }
    else if (lowerInput.contains("bright") || lowerInput.contains("明るい") || lowerInput.contains("happy"))
    {
        cmd.mood = "bright";
    }
    else if (lowerInput.contains("tense") || lowerInput.contains("緊張"))
    {
        cmd.mood = "tense";
    }
    else
    {
        cmd.mood = "neutral";
    }

    // 3. Detect Bars (Simple parsing for "1-4", "1~8" etc)
    // This is a very basic implementation.
    // Regex would be better but keeping it simple with standard string functions for now or use std::regex if needed.
    // Let's try to find numbers.
    
    // Default values
    cmd.barsStart = 1;
    cmd.barsEnd = 4;

    // TODO: Implement more robust bar parsing
    
    // 4. Intensity
    if (lowerInput.contains("strong") || lowerInput.contains("激しい"))
    {
        cmd.intensity = 0.8f;
    }
    else if (lowerInput.contains("soft") || lowerInput.contains("優しい"))
    {
        cmd.intensity = 0.3f;
    }
    else
    {
        cmd.intensity = 0.5f;
    }

    if (cmd.type != AgentCommand::Type::Unknown)
    {
        commands.add(cmd);
    }

    return commands;
}
