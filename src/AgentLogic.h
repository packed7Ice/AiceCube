#pragma once
#include "AgentCommand.h"

class AgentLogic
{
public:
    static juce::Array<AgentCommand> interpretInstruction(const juce::String& inputText);
};
