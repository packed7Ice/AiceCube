#pragma once
#include <juce_core/juce_core.h>
#include <functional>
#include "model/ProjectState.h"
#include "ApiClient.h"
#include "AgentCommand.h"

class CommandExecutor
{
public:
    CommandExecutor(ProjectState& state, ApiClient& client);
    ~CommandExecutor() = default;

    void execute(const AgentCommand& cmd, std::function<void(const juce::String&)> logCallback, std::function<void()> completeCallback);

private:
    ProjectState& projectState;
    ApiClient& apiClient;
};
