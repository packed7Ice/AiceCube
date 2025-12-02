#pragma once
#include <juce_core/juce_core.h>
#include "AgentCommand.h"
#include "model/MusicData.h"

class ApiClient : public juce::Thread
{
public:
    ApiClient();
    ~ApiClient() override;

    void generateMelody(const AgentCommand& cmd, std::function<void(const Sequence&)> callback);

    void run() override;

private:
    struct PendingRequest
    {
        AgentCommand cmd;
        std::function<void(const Sequence&)> callback;
    };

    juce::CriticalSection lock;
    juce::Array<PendingRequest> requestQueue;
    
    // Helper to perform the actual HTTP request
    void performRequest(const PendingRequest& req);
};
