#include "MessageHandler.h"
#include "../src/model/ProjectState.h"
#include <iostream>

namespace ipc {

MessageHandler::MessageHandler(ProjectState& projectState)
    : projectState_(projectState)
{
    registerBuiltinHandlers();
}

void MessageHandler::registerBuiltinHandlers() {
    // Transport handlers
    handlers_[CommandType::TRANSPORT_PLAY] = [this](const Command& cmd) { return handleTransportPlay(cmd); };
    handlers_[CommandType::TRANSPORT_STOP] = [this](const Command& cmd) { return handleTransportStop(cmd); };
    handlers_[CommandType::TRANSPORT_SET_TEMPO] = [this](const Command& cmd) { return handleTransportSetTempo(cmd); };
    handlers_[CommandType::TRANSPORT_SET_LOOP] = [this](const Command& cmd) { return handleTransportSetLoop(cmd); };
    handlers_[CommandType::TRANSPORT_SET_PLAYHEAD] = [this](const Command& cmd) { return handleTransportSetPlayhead(cmd); };
    handlers_[CommandType::TRANSPORT_TOGGLE_METRONOME] = [this](const Command& cmd) { return handleTransportToggleMetronome(cmd); };
    
    // Project handlers
    handlers_[CommandType::PROJECT_NEW] = [this](const Command& cmd) { return handleProjectNew(cmd); };
    handlers_[CommandType::PROJECT_SAVE] = [this](const Command& cmd) { return handleProjectSave(cmd); };
    handlers_[CommandType::PROJECT_OPEN] = [this](const Command& cmd) { return handleProjectOpen(cmd); };
    
    // Track handlers
    handlers_[CommandType::TRACK_CREATE] = [this](const Command& cmd) { return handleTrackCreate(cmd); };
    handlers_[CommandType::TRACK_DELETE] = [this](const Command& cmd) { return handleTrackDelete(cmd); };
    handlers_[CommandType::TRACK_RENAME] = [this](const Command& cmd) { return handleTrackRename(cmd); };
    
    // Mixer handlers
    handlers_[CommandType::MIXER_SET_VOLUME] = [this](const Command& cmd) { return handleMixerSetVolume(cmd); };
    handlers_[CommandType::MIXER_SET_PAN] = [this](const Command& cmd) { return handleMixerSetPan(cmd); };
    handlers_[CommandType::MIXER_TOGGLE_MUTE] = [this](const Command& cmd) { return handleMixerToggleMute(cmd); };
    handlers_[CommandType::MIXER_TOGGLE_SOLO] = [this](const Command& cmd) { return handleMixerToggleSolo(cmd); };
}

void MessageHandler::registerHandler(const std::string& commandType, CommandHandler handler) {
    handlers_[commandType] = handler;
}

std::string MessageHandler::processMessage(const std::string& jsonMessage) {
    try {
        json j = json::parse(jsonMessage);
        
        // Validate message type
        std::string messageType = j.value("type", "");
        if (messageType != "command") {
            return Event{"error", "Invalid message type", {}}.toJson().dump();
        }
        
        Command cmd = Command::fromJson(j);
        
        std::cout << "[Handler] Processing command: " << cmd.type << std::endl;
        
        // Find and execute handler
        auto it = handlers_.find(cmd.type);
        if (it != handlers_.end()) {
            json result = it->second(cmd);
            
            // Return response with command ID
            json response = {
                {"type", "response"},
                {"id", cmd.id},
                {"success", true},
                {"data", result}
            };
            return response.dump();
        }
        else {
            std::cerr << "[Handler] Unknown command: " << cmd.type << std::endl;
            json response = {
                {"type", "response"},
                {"id", cmd.id},
                {"success", false},
                {"error", "Unknown command: " + cmd.type}
            };
            return response.dump();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[Handler] Error processing message: " << e.what() << std::endl;
        return Event{"error", e.what(), {}}.toJson().dump();
    }
}

TransportState MessageHandler::getTransportState() const {
    TransportState state;
    state.isPlaying = projectState_.isPlaying;
    state.playheadBeat = projectState_.playheadBeat;
    state.tempo = projectState_.tempo;
    state.isLooping = projectState_.isLooping;
    state.loopStart = projectState_.loopStart;
    state.loopEnd = projectState_.loopEnd;
    state.metronomeEnabled = projectState_.metronomeEnabled;
    state.timeSignatureNumerator = projectState_.timeSignatureNumerator;
    state.timeSignatureDenominator = projectState_.timeSignatureDenominator;
    return state;
}

json MessageHandler::getProjectState() const {
    json tracks = json::array();
    for (const auto& track : projectState_.tracks) {
        json clips = json::array();
        for (const auto& clip : track->clips) {
            clips.push_back({
                {"startBeat", clip.startBeat},
                {"lengthBeats", clip.lengthBeats},
                {"name", clip.name.toStdString()},
                {"isMidi", clip.isMidi},
                {"color", clip.clipColor.toString().toStdString()}
            });
        }
        
        tracks.push_back({
            {"id", track->id.toString().toStdString()},
            {"type", static_cast<int>(track->type)},
            {"name", track->name.toStdString()},
            {"volume", track->volume},
            {"pan", track->pan},
            {"mute", track->mute},
            {"solo", track->solo},
            {"arm", track->arm},
            {"color", track->trackColor.toString().toStdString()},
            {"clips", clips}
        });
    }
    
    return {
        {"tracks", tracks},
        {"selectedTrackIndex", projectState_.selectedTrackIndex}
    };
}

//==============================================================================
// Transport Handlers
//==============================================================================

json MessageHandler::handleTransportPlay(const Command& cmd) {
    projectState_.isPlaying = true;
    std::cout << "[Transport] Play" << std::endl;
    return getTransportState().toJson();
}

json MessageHandler::handleTransportStop(const Command& cmd) {
    projectState_.isPlaying = false;
    std::cout << "[Transport] Stop" << std::endl;
    return getTransportState().toJson();
}

json MessageHandler::handleTransportSetTempo(const Command& cmd) {
    double tempo = cmd.payload.value("tempo", 120.0);
    projectState_.tempo = std::clamp(tempo, 20.0, 999.0);
    std::cout << "[Transport] Set tempo: " << projectState_.tempo << std::endl;
    return getTransportState().toJson();
}

json MessageHandler::handleTransportSetLoop(const Command& cmd) {
    projectState_.isLooping = cmd.payload.value("enabled", false);
    projectState_.loopStart = cmd.payload.value("start", 0.0);
    projectState_.loopEnd = cmd.payload.value("end", 16.0);
    std::cout << "[Transport] Set loop: " << projectState_.isLooping 
              << " [" << projectState_.loopStart << " - " << projectState_.loopEnd << "]" << std::endl;
    return getTransportState().toJson();
}

json MessageHandler::handleTransportSetPlayhead(const Command& cmd) {
    projectState_.playheadBeat = cmd.payload.value("beat", 0.0);
    std::cout << "[Transport] Set playhead: " << projectState_.playheadBeat << std::endl;
    return getTransportState().toJson();
}

json MessageHandler::handleTransportToggleMetronome(const Command& cmd) {
    projectState_.metronomeEnabled = !projectState_.metronomeEnabled;
    std::cout << "[Transport] Metronome: " << (projectState_.metronomeEnabled ? "ON" : "OFF") << std::endl;
    return getTransportState().toJson();
}

//==============================================================================
// Project Handlers
//==============================================================================

json MessageHandler::handleProjectNew(const Command& cmd) {
    // Clear all tracks
    projectState_.tracks.clear();
    projectState_.selectedTrackIndex = -1;
    projectState_.playheadBeat = 0.0;
    projectState_.isPlaying = false;
    
    std::cout << "[Project] New project created" << std::endl;
    return getProjectState();
}

json MessageHandler::handleProjectSave(const Command& cmd) {
    // TODO: Implement file saving
    std::string path = cmd.payload.value("path", "");
    std::cout << "[Project] Save to: " << path << std::endl;
    return {{"saved", true}, {"path", path}};
}

json MessageHandler::handleProjectOpen(const Command& cmd) {
    // TODO: Implement file loading
    std::string path = cmd.payload.value("path", "");
    std::cout << "[Project] Open: " << path << std::endl;
    return getProjectState();
}

//==============================================================================
// Track Handlers
//==============================================================================

json MessageHandler::handleTrackCreate(const Command& cmd) {
    std::string typeStr = cmd.payload.value("type", "midi");
    std::string name = cmd.payload.value("name", "New Track");
    
    TrackType type = TrackType::Midi;
    if (typeStr == "audio") type = TrackType::Audio;
    else if (typeStr == "bus") type = TrackType::Bus;
    else if (typeStr == "master") type = TrackType::Master;
    
    auto track = projectState_.addTrack(type, juce::String(name));
    
    std::cout << "[Track] Created: " << name << std::endl;
    return getProjectState();
}

json MessageHandler::handleTrackDelete(const Command& cmd) {
    int index = cmd.payload.value("index", -1);
    if (index >= 0 && index < static_cast<int>(projectState_.tracks.size())) {
        projectState_.removeTrack(index);
        std::cout << "[Track] Deleted index: " << index << std::endl;
    }
    return getProjectState();
}

json MessageHandler::handleTrackRename(const Command& cmd) {
    int index = cmd.payload.value("index", -1);
    std::string name = cmd.payload.value("name", "");
    
    if (auto* track = projectState_.getTrack(index)) {
        track->name = juce::String(name);
        std::cout << "[Track] Renamed to: " << name << std::endl;
    }
    return getProjectState();
}

//==============================================================================
// Mixer Handlers
//==============================================================================

json MessageHandler::handleMixerSetVolume(const Command& cmd) {
    int index = cmd.payload.value("trackIndex", -1);
    float volume = cmd.payload.value("volume", 1.0f);
    
    if (auto* track = projectState_.getTrack(index)) {
        track->volume = std::clamp(volume, 0.0f, 2.0f);
        std::cout << "[Mixer] Track " << index << " volume: " << track->volume << std::endl;
    }
    return json::object();
}

json MessageHandler::handleMixerSetPan(const Command& cmd) {
    int index = cmd.payload.value("trackIndex", -1);
    float pan = cmd.payload.value("pan", 0.0f);
    
    if (auto* track = projectState_.getTrack(index)) {
        track->pan = std::clamp(pan, -1.0f, 1.0f);
        std::cout << "[Mixer] Track " << index << " pan: " << track->pan << std::endl;
    }
    return json::object();
}

json MessageHandler::handleMixerToggleMute(const Command& cmd) {
    int index = cmd.payload.value("trackIndex", -1);
    
    if (auto* track = projectState_.getTrack(index)) {
        track->mute = !track->mute;
        std::cout << "[Mixer] Track " << index << " mute: " << track->mute << std::endl;
    }
    return json::object();
}

json MessageHandler::handleMixerToggleSolo(const Command& cmd) {
    int index = cmd.payload.value("trackIndex", -1);
    
    if (auto* track = projectState_.getTrack(index)) {
        track->solo = !track->solo;
        std::cout << "[Mixer] Track " << index << " solo: " << track->solo << std::endl;
    }
    return json::object();
}

} // namespace ipc
