#include "CommandExecutor.h"

CommandExecutor::CommandExecutor(ProjectState& state, ApiClient& client)
    : projectState(state), apiClient(client)
{
}

void CommandExecutor::execute(const AgentCommand& cmd, std::function<void(const juce::String&)> logCallback, std::function<void()> completeCallback)
{
    if (cmd.action == AgentAction::Generate || cmd.action == AgentAction::Replace || cmd.action == AgentAction::Extend)
    {
        apiClient.generateMelody(cmd, [=](const Sequence& seq) {
            
            // Convert Sequence to Clip
            Clip newClip;
            newClip.startBeat = 0.0; // Simplify for now
            newClip.lengthBeats = 4.0; // Default
            newClip.name = "Generated Clip";
            newClip.isMidi = true;
            
            for (const auto& n : seq.notes)
            {
                auto message = juce::MidiMessage::noteOn(1, n.pitch, (float)n.velocity / 127.0f);
                newClip.midiSequence.addEvent(message, n.startTime);
                auto off = juce::MidiMessage::noteOff(1, n.pitch);
                newClip.midiSequence.addEvent(off, n.startTime + n.duration);
            }
            newClip.midiSequence.updateMatchedPairs();
            
            // Find or create track
            int trackIndex = -1;
            
            // 1. Use selected track if available
            if (projectState.selectedTrackIndex >= 0 && projectState.selectedTrackIndex < projectState.tracks.size())
            {
                trackIndex = projectState.selectedTrackIndex;
            }
            else
            {
                // 2. Fallback to name search
                juce::String targetName = cmd.targetTrackName;
                
                for (int i = 0; i < projectState.tracks.size(); ++i)
                {
                    if (projectState.tracks[i]->name == targetName)
                    {
                        trackIndex = i;
                        break;
                    }
                }
                
                if (trackIndex == -1)
                {
                    projectState.addTrack(TrackType::Midi, targetName);
                    trackIndex = (int)projectState.tracks.size() - 1;
                }
            }
            
            // Add Clip
            projectState.addClip(trackIndex, newClip);
            
            if (completeCallback) completeCallback();
        });
    }
}
