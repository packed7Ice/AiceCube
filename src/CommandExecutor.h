#pragma once
#include <juce_core/juce_core.h>
#include <functional>
#include "AppState.h"
#include "ApiClient.h"
#include "AgentCommand.h"

class CommandExecutor
{
public:
    CommandExecutor(AppState& state, ApiClient& client);
    ~CommandExecutor() = default;

    void execute(const AgentCommand& cmd, std::function<void(const juce::String&)> logCallback, std::function<void()> completeCallback);

private:
    AppState& appState;
    ApiClient& apiClient;
};
