#pragma once

#include <string>
#include <functional>
#include <nlohmann/json.hpp>

namespace ipc {

using json = nlohmann::json;

//==============================================================================
// Command Types (UI → Engine)
//==============================================================================
namespace CommandType {
    // Transport
    constexpr const char* TRANSPORT_PLAY = "transport.play";
    constexpr const char* TRANSPORT_STOP = "transport.stop";
    constexpr const char* TRANSPORT_SET_TEMPO = "transport.setTempo";
    constexpr const char* TRANSPORT_SET_LOOP = "transport.setLoop";
    constexpr const char* TRANSPORT_SET_PLAYHEAD = "transport.setPlayhead";
    constexpr const char* TRANSPORT_TOGGLE_METRONOME = "transport.toggleMetronome";
    
    // Project
    constexpr const char* PROJECT_NEW = "project.new";
    constexpr const char* PROJECT_SAVE = "project.save";
    constexpr const char* PROJECT_OPEN = "project.open";
    
    // Track
    constexpr const char* TRACK_CREATE = "track.create";
    constexpr const char* TRACK_DELETE = "track.delete";
    constexpr const char* TRACK_RENAME = "track.rename";
    constexpr const char* TRACK_SET_COLOR = "track.setColor";
    
    // Clip
    constexpr const char* CLIP_CREATE_MIDI = "clip.createMidi";
    constexpr const char* CLIP_MOVE = "clip.move";
    constexpr const char* CLIP_RESIZE = "clip.resize";
    constexpr const char* CLIP_DUPLICATE = "clip.duplicate";
    constexpr const char* CLIP_DELETE = "clip.delete";
    
    // Mixer
    constexpr const char* MIXER_SET_VOLUME = "mixer.setVolume";
    constexpr const char* MIXER_SET_PAN = "mixer.setPan";
    constexpr const char* MIXER_TOGGLE_MUTE = "mixer.toggleMute";
    constexpr const char* MIXER_TOGGLE_SOLO = "mixer.toggleSolo";
}

//==============================================================================
// State Types (Engine → UI)
//==============================================================================
namespace StateType {
    constexpr const char* TRANSPORT = "transport";
    constexpr const char* PROJECT = "project";
    constexpr const char* METERS = "meters";
}

//==============================================================================
// Event Types (Engine → UI)
//==============================================================================
namespace EventType {
    constexpr const char* ERROR = "error";
    constexpr const char* TOAST = "toast";
    constexpr const char* FILE_IMPORTED = "fileImported";
    constexpr const char* RENDER_PROGRESS = "renderProgress";
    constexpr const char* PLUGIN_SCAN_PROGRESS = "pluginScanProgress";
}

//==============================================================================
// Message Structures
//==============================================================================

// Incoming command from UI
struct Command {
    std::string id;      // Unique request ID
    std::string type;    // Command type (e.g., "transport.play")
    json payload;        // Command-specific data
    
    static Command fromJson(const json& j) {
        Command cmd;
        cmd.id = j.value("id", "");
        cmd.type = j.value("command", "");
        cmd.payload = j.value("payload", json::object());
        return cmd;
    }
};

// Outgoing state update to UI
struct StateUpdate {
    std::string scope;   // State scope (e.g., "transport")
    json data;           // State data
    
    json toJson() const {
        return {
            {"type", "state"},
            {"scope", scope},
            {"data", data}
        };
    }
};

// Outgoing event to UI
struct Event {
    std::string eventType;
    std::string message;
    json data;
    
    json toJson() const {
        return {
            {"type", "event"},
            {"event", eventType},
            {"message", message},
            {"data", data}
        };
    }
};

//==============================================================================
// Transport State (frequently sent)
//==============================================================================
struct TransportState {
    bool isPlaying = false;
    double playheadBeat = 0.0;
    double tempo = 120.0;
    bool isLooping = false;
    double loopStart = 0.0;
    double loopEnd = 16.0;
    bool metronomeEnabled = false;
    int timeSignatureNumerator = 4;
    int timeSignatureDenominator = 4;
    
    json toJson() const {
        return {
            {"isPlaying", isPlaying},
            {"playheadBeat", playheadBeat},
            {"tempo", tempo},
            {"isLooping", isLooping},
            {"loopStart", loopStart},
            {"loopEnd", loopEnd},
            {"metronomeEnabled", metronomeEnabled},
            {"timeSignatureNumerator", timeSignatureNumerator},
            {"timeSignatureDenominator", timeSignatureDenominator}
        };
    }
};

} // namespace ipc
