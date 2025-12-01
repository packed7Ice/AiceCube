#include "CommandExecutor.h"

CommandExecutor::CommandExecutor(AppState& state, ApiClient& client)
    : appState(state), apiClient(client)
{
}

void CommandExecutor::execute(const AgentCommand& cmd, std::function<void(const juce::String&)> logCallback, std::function<void()> completeCallback)
{
    if (cmd.action == AgentAction::Generate || cmd.action == AgentAction::Replace || cmd.action == AgentAction::Extend)
    {
        // logCallback("Sending request to server for " + cmd.targetTrackName + " (" + cmd.style + ")...");
        
        apiClient.generateMelody(cmd, [=](const Sequence& seq) {
            // logCallback("Received " + juce::String(seq.notes.size()) + " notes from server.");
            
            // Convert Sequence to Clip
            Clip newClip;
            newClip.notes = seq.notes;
            newClip.startBeat = (double)(cmd.barsStart - 1) * 4.0; // Assume 4/4
            newClip.lengthBeats = (double)(cmd.barsEnd - cmd.barsStart + 1) * 4.0;
            
            // Find or create track
            int trackIndex = -1;
            juce::String targetName = cmd.targetTrackName.isNotEmpty() ? cmd.targetTrackName : "Melody";
            
            for (int i = 0; i < appState.tracks.size(); ++i)
            {
                if (appState.tracks[i].name.equalsIgnoreCase(targetName))
                {
                    trackIndex = i;
                    break;
                }
            }
            
            if (trackIndex == -1)
            {
                appState.addTrack(targetName);
                trackIndex = appState.tracks.size() - 1;
                // logCallback("Created new track: " + targetName);
            }
            
            // If Replace, clear existing clips in range (simplified: clear all for now or just add)
            if (cmd.action == AgentAction::Replace)
            {
                appState.tracks.getReference(trackIndex).clips.clear(); // Simple clear
            }

            appState.addClip(trackIndex, newClip);
            // logCallback("Added clip to track " + targetName);
            
            if (completeCallback) completeCallback();
        });
    }
    else if (cmd.action == AgentAction::ClearRegion)
    {
        // Implement Clear
        // logCallback("Clear command not fully implemented yet.");
    }
    else
    {
        // logCallback("Command action not yet implemented.");
    }
}
