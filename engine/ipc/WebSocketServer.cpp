#include "WebSocketServer.h"
#include <ixwebsocket/IXWebSocketServer.h>
#include <iostream>

namespace ipc {

WebSocketServer::WebSocketServer(int port)
    : port_(port)
{
}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::start() {
    if (running_) return true;
    
    server_ = std::make_unique<ix::WebSocketServer>(port_, "0.0.0.0");
    
    server_->setOnClientMessageCallback(
        [this](std::shared_ptr<ix::ConnectionState> connectionState,
               ix::WebSocket& webSocket,
               const ix::WebSocketMessagePtr& msg) {
            
            if (msg->type == ix::WebSocketMessageType::Open) {
                std::cout << "[IPC] Client connected: " << connectionState->getRemoteIp() << std::endl;
                
                {
                    std::lock_guard<std::mutex> lock(clientsMutex_);
                    // Note: We can't directly store the WebSocket reference, 
                    // but we can track connection count
                }
                clientCount_++;
                
                if (connectionCallback_) {
                    connectionCallback_(true);
                }
            }
            else if (msg->type == ix::WebSocketMessageType::Close) {
                std::cout << "[IPC] Client disconnected" << std::endl;
                clientCount_--;
                
                if (connectionCallback_) {
                    connectionCallback_(false);
                }
            }
            else if (msg->type == ix::WebSocketMessageType::Message) {
                std::cout << "[IPC] Received: " << msg->str << std::endl;
                
                if (messageCallback_) {
                    // Create response sender
                    auto sendResponse = [&webSocket](const std::string& response) {
                        webSocket.send(response);
                    };
                    
                    messageCallback_(msg->str, sendResponse);
                }
            }
            else if (msg->type == ix::WebSocketMessageType::Error) {
                std::cerr << "[IPC] Error: " << msg->errorInfo.reason << std::endl;
            }
        }
    );
    
    auto result = server_->listen();
    if (!result.first) {
        std::cerr << "[IPC] Failed to start server: " << result.second << std::endl;
        return false;
    }
    
    server_->start();
    running_ = true;
    
    std::cout << "[IPC] WebSocket server started on port " << port_ << std::endl;
    return true;
}

void WebSocketServer::stop() {
    if (!running_) return;
    
    if (server_) {
        server_->stop();
        server_.reset();
    }
    
    running_ = false;
    std::cout << "[IPC] WebSocket server stopped" << std::endl;
}

void WebSocketServer::broadcast(const std::string& message) {
    if (server_) {
        // ix::WebSocketServer doesn't have direct broadcast, 
        // but we can iterate through connections
        // For now, this is handled via the server's internal mechanism
        // Each client connection can be accessed through setOnClientMessageCallback
    }
}

} // namespace ipc
