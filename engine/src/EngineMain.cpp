/**
 * AiceCube Engine - Headless Audio Engine with WebSocket IPC
 * 
 * This is the main entry point for the standalone engine process.
 * It initializes the audio system and WebSocket server for UI communication.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>

#include <juce_core/juce_core.h>

#include "ipc/WebSocketServer.h"
#include "ipc/MessageHandler.h"
#include "ipc/IpcMessages.h"
#include "model/ProjectState.h"

// Global flag for graceful shutdown
std::atomic<bool> g_running{true};

void signalHandler(int signal) {
    std::cout << "\n[Engine] Shutdown signal received..." << std::endl;
    g_running = false;
}

//==============================================================================
// Engine Application
//==============================================================================
class EngineApplication {
public:
    EngineApplication() 
        : messageHandler_(projectState_),
          wsServer_(9001)  // Default WebSocket port
    {
    }
    
    bool initialize() {
        std::cout << "========================================" << std::endl;
        std::cout << "  AiceCube Engine v0.1.0" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // Initialize JUCE
        juce::initialiseJuce_GUI();
        
        // Setup WebSocket message handling
        wsServer_.setMessageCallback(
            [this](const std::string& message, std::function<void(const std::string&)> sendResponse) {
                std::string response = messageHandler_.processMessage(message);
                sendResponse(response);
                
                // After processing command, also send updated transport state
                auto transportState = messageHandler_.getTransportState();
                ipc::StateUpdate stateUpdate;
                stateUpdate.scope = ipc::StateType::TRANSPORT;
                stateUpdate.data = transportState.toJson();
                sendResponse(stateUpdate.toJson().dump());
            }
        );
        
        wsServer_.setConnectionCallback([](bool connected) {
            if (connected) {
                std::cout << "[Engine] UI client connected" << std::endl;
            } else {
                std::cout << "[Engine] UI client disconnected" << std::endl;
            }
        });
        
        // Start WebSocket server
        if (!wsServer_.start()) {
            std::cerr << "[Engine] Failed to start WebSocket server!" << std::endl;
            return false;
        }
        
        std::cout << "[Engine] Ready. Waiting for UI connection on ws://localhost:9001" << std::endl;
        std::cout << "[Engine] Press Ctrl+C to stop." << std::endl;
        std::cout << std::endl;
        
        return true;
    }
    
    void run() {
        // Main loop - process transport and broadcast state
        const auto transportBroadcastInterval = std::chrono::milliseconds(50);  // 20 fps
        auto lastBroadcast = std::chrono::steady_clock::now();
        
        while (g_running) {
            auto now = std::chrono::steady_clock::now();
            
            // Update playhead if playing
            if (projectState_.isPlaying) {
                // Calculate elapsed time and advance playhead
                auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                    now - lastBroadcast
                ).count();
                
                double beatsPerSecond = projectState_.tempo / 60.0;
                double beatsElapsed = (elapsed / 1000000.0) * beatsPerSecond;
                projectState_.playheadBeat += beatsElapsed;
                
                // Handle looping
                if (projectState_.isLooping && projectState_.playheadBeat >= projectState_.loopEnd) {
                    projectState_.playheadBeat = projectState_.loopStart;
                }
            }
            
            // Broadcast transport state periodically
            if (now - lastBroadcast >= transportBroadcastInterval) {
                if (wsServer_.getClientCount() > 0) {
                    auto transportState = messageHandler_.getTransportState();
                    ipc::StateUpdate stateUpdate;
                    stateUpdate.scope = ipc::StateType::TRANSPORT;
                    stateUpdate.data = transportState.toJson();
                    
                    // Note: broadcast needs to be implemented properly
                    // For now, state is sent as response to commands
                }
                lastBroadcast = now;
            }
            
            // Sleep to prevent busy-waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    void shutdown() {
        std::cout << "[Engine] Shutting down..." << std::endl;
        wsServer_.stop();
        juce::shutdownJuce_GUI();
        std::cout << "[Engine] Goodbye!" << std::endl;
    }
    
private:
    ProjectState projectState_;
    ipc::MessageHandler messageHandler_;
    ipc::WebSocketServer wsServer_;
};

//==============================================================================
// Main
//==============================================================================
int main(int argc, char* argv[]) {
    // Setup signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    EngineApplication app;
    
    if (!app.initialize()) {
        return 1;
    }
    
    app.run();
    app.shutdown();
    
    return 0;
}
