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
    // Construct JSON body
    juce::DynamicObject* jsonBody = new juce::DynamicObject();
    jsonBody->setProperty("type", "AddMelody"); // Simplified
    jsonBody->setProperty("track", req.cmd.track);
    jsonBody->setProperty("barsStart", req.cmd.barsStart);
    jsonBody->setProperty("barsEnd", req.cmd.barsEnd);
    jsonBody->setProperty("mood", req.cmd.mood);
    jsonBody->setProperty("intensity", req.cmd.intensity);
    if (req.cmd.extra.isNotEmpty())
        jsonBody->setProperty("extra", req.cmd.extra);

    juce::var jsonVar(jsonBody);
    juce::String jsonString = juce::JSON::toString(jsonVar);

    // Setup URL
    juce::URL url("http://127.0.0.1:8000/generate_melody");
    
    // Perform POST
    juce::StringPairArray responseHeaders;
    int statusCode = 0;
    
    // Note: withPOSTData is deprecated in newer JUCE versions in favor of other methods, 
    // but let's stick to standard ways. 
    // actually `withPOSTData` returns a new URL.
    
    auto postUrl = url.withPOSTData(jsonString);
    
    // We need to set Content-Type header manually if we use withPOSTData with string?
    // Actually `readEntireTextStream` allows extra headers.
    
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
                    n.start = noteObj["start"];
                    n.duration = noteObj["duration"];
                    n.velocity = noteObj["velocity"];
                    seq.notes.add(n);
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
        juce::String errorMsg = "Request failed. Status: " + juce::String(statusCode);
        // We might want to log this or callback with empty sequence
        juce::MessageManager::callAsync([cb = req.callback]() {
            if (cb) cb(Sequence()); // Empty sequence indicates failure
        });
    }
}
