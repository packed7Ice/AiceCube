#include "CommandExecutor.h"

CommandExecutor::CommandExecutor(AppState& state, ApiClient& client)
    : appState(state), apiClient(client)
{
}

void CommandExecutor::execute(const AgentCommand& cmd, std::function<void(const juce::String&)> onLog, std::function<void()> onComplete)
{
    if (cmd.type == AgentCommand::Type::AddMelody || cmd.type == AgentCommand::Type::AddDrums)
    {
        onLog("Sending request to server for " + cmd.targetTrackName + "...");
        
        apiClient.generateMelody(cmd, [this, cmd, onLog, onComplete](const Sequence& seq) {
            onLog("Received " + juce::String(seq.notes.size()) + " notes from server.");
            
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
                onLog("Created new track: " + targetName);
            }
            
            appState.addClip(trackIndex, newClip);
            onLog("Added clip to track " + targetName);
            
            if (onComplete) onComplete();
        });
    }
    else
    {
        onLog("Command type not yet implemented.");
    }
}
