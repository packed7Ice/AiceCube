#pragma once

#include "IpcMessages.h"
#include <functional>
#include <map>

// Forward declaration
class ProjectState;

namespace ipc {

//==============================================================================
// Message Handler - parses commands and dispatches to appropriate handlers
//==============================================================================
class MessageHandler {
public:
    using CommandHandler = std::function<json(const Command& cmd)>;
    
    MessageHandler(ProjectState& projectState);
    
    // Process incoming JSON message
    std::string processMessage(const std::string& jsonMessage);
    
    // Register custom command handler
    void registerHandler(const std::string& commandType, CommandHandler handler);
    
    // State getters for broadcasting
    TransportState getTransportState() const;
    json getProjectState() const;
    
private:
    ProjectState& projectState_;
    std::map<std::string, CommandHandler> handlers_;
    
    // Register all built-in handlers
    void registerBuiltinHandlers();
    
    // Built-in command handlers
    json handleTransportPlay(const Command& cmd);
    json handleTransportStop(const Command& cmd);
    json handleTransportSetTempo(const Command& cmd);
    json handleTransportSetLoop(const Command& cmd);
    json handleTransportSetPlayhead(const Command& cmd);
    json handleTransportToggleMetronome(const Command& cmd);
    
    json handleProjectNew(const Command& cmd);
    json handleProjectSave(const Command& cmd);
    json handleProjectOpen(const Command& cmd);
    
    json handleTrackCreate(const Command& cmd);
    json handleTrackDelete(const Command& cmd);
    json handleTrackRename(const Command& cmd);
    
    json handleMixerSetVolume(const Command& cmd);
    json handleMixerSetPan(const Command& cmd);
    json handleMixerToggleMute(const Command& cmd);
    json handleMixerToggleSolo(const Command& cmd);
};

} // namespace ipc
