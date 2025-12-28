#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>

// Forward declaration
namespace ix { class WebSocket; class WebSocketServer; }

namespace ipc {

//==============================================================================
// WebSocket Server for Engine ↔ UI communication
//==============================================================================
class WebSocketServer {
public:
    using MessageCallback = std::function<void(const std::string& message, 
                                                std::function<void(const std::string&)> sendResponse)>;
    using ConnectionCallback = std::function<void(bool connected)>;
    
    WebSocketServer(int port = 9001);
    ~WebSocketServer();
    
    // Server control
    bool start();
    void stop();
    bool isRunning() const { return running_; }
    
    // Callbacks
    void setMessageCallback(MessageCallback callback) { messageCallback_ = callback; }
    void setConnectionCallback(ConnectionCallback callback) { connectionCallback_ = callback; }
    
    // Broadcast message to all connected clients
    void broadcast(const std::string& message);
    
    // Get connection status
    int getClientCount() const { return clientCount_.load(); }
    
private:
    int port_;
    std::atomic<bool> running_{false};
    std::atomic<int> clientCount_{0};
    
    std::unique_ptr<ix::WebSocketServer> server_;
    std::mutex clientsMutex_;
    std::vector<std::shared_ptr<ix::WebSocket>> clients_;
    
    MessageCallback messageCallback_;
    ConnectionCallback connectionCallback_;
};

} // namespace ipc
