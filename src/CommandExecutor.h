#pragma once
#include <juce_core/juce_core.h>
#include "AppState.h"
#include "ApiClient.h"
#include "AgentCommand.h"

class CommandExecutor
{
public:
    CommandExecutor(AppState& state, ApiClient& client);
    ~CommandExecutor() = default;

    void execute(const AgentCommand& cmd, std::function<void(const juce::String&)> onLog, std::function<void()> onComplete);

private:
    AppState& appState;
    ApiClient& apiClient;
};
