#include "ApiClient.h"
#include <juce_events/juce_events.h>

ApiClient::ApiClient() : Thread("ApiClientThread")
{
    startThread();
}

ApiClient::~ApiClient()
{
    stopThread(2000);
}

void ApiClient::generateMelody(const AgentCommand& cmd, std::function<void(const Sequence&)> callback)
{
    juce::ScopedLock sl(lock);
    requestQueue.add({cmd, callback});
    notify();
}

void ApiClient::run()
{
    while (!threadShouldExit())
    {
        PendingRequest req;
        {
            juce::ScopedLock sl(lock);
            if (requestQueue.isEmpty())
            {
                wait(500);
                continue;
            }
            req = requestQueue.removeAndReturn(0);
        }

        performRequest(req);
    }
}

void ApiClient::performRequest(const PendingRequest& req)
{
    // Construct JSON object
    juce::DynamicObject* jsonBody = new juce::DynamicObject();
    
    // Map Role to String
    juce::String roleStr = "melody";
    switch (req.cmd.role) {
        case MusicalRole::Drums: roleStr = "drums"; break;
        case MusicalRole::Bass: roleStr = "bass"; break;
        case MusicalRole::Chords: roleStr = "chords"; break;
        case MusicalRole::Pad: roleStr = "pad"; break;
        case MusicalRole::FX: roleStr = "fx"; break;
        default: break;
    }
    jsonBody->setProperty("role", roleStr);

    // Map Action to String
    juce::String actionStr = "generate";
    switch (req.cmd.action) {
        case AgentAction::Replace: actionStr = "replace"; break;
        case AgentAction::Extend: actionStr = "extend"; break;
        case AgentAction::ClearRegion: actionStr = "clear"; break;
        case AgentAction::Humanize: actionStr = "humanize"; break;
        default: break;
    }
    jsonBody->setProperty("action", actionStr);

    jsonBody->setProperty("barsStart", req.cmd.barsStart);
    jsonBody->setProperty("barsEnd", req.cmd.barsEnd);
    jsonBody->setProperty("style", req.cmd.style);
    jsonBody->setProperty("density", req.cmd.density);
    jsonBody->setProperty("tempo", 120); // Should get from AppState if possible

    juce::var jsonVar(jsonBody);
    juce::String jsonString = juce::JSON::toString(jsonVar);

    // Setup URL
    juce::URL url("http://127.0.0.1:8000/generate"); // Updated endpoint
    
    // Perform POST
    juce::StringPairArray responseHeaders;
    int statusCode = 0;
    
    auto postUrl = url.withPOSTData(jsonString);
    
    std::unique_ptr<juce::InputStream> stream = postUrl.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
            .withExtraHeaders("Content-Type: application/json")
            .withConnectionTimeoutMs(2000)
            .withResponseHeaders(&responseHeaders)
            .withStatusCode(&statusCode)
    );

    if (stream != nullptr && statusCode == 200)
    {
        juce::String responseText = stream->readEntireStreamAsString();
        
        // Parse Response
        auto responseVar = juce::JSON::parse(responseText);
        if (responseVar.hasProperty("notes"))
        {
            Sequence seq;
            auto notesArray = responseVar["notes"];
            if (notesArray.isArray())
            {
                for (int i = 0; i < notesArray.size(); ++i)
                {
                    auto noteObj = notesArray[i];
                    Note n;
                    n.pitch = noteObj["pitch"];
                    n.startTime = noteObj["start"];
                    n.duration = noteObj["duration"];
                    n.velocity = noteObj["velocity"];
                    seq.notes.push_back(n);
                }
            }
            
            // Callback on Message Thread
            juce::MessageManager::callAsync([cb = req.callback, s = seq]() {
                if (cb) cb(s);
            });
        }
    }
    else
    {
        // Error handling
        juce::MessageManager::callAsync([cb = req.callback]() {
            if (cb) cb(Sequence()); // Empty sequence indicates failure
        });
    }
}
