cmake_minimum_required(VERSION 3.15)

project(AiceCube_Engine VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    add_compile_options(/utf-8)
endif()

# Fetch IXWebSocket
include(FetchContent)

FetchContent_Declare(
    ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
    GIT_TAG v11.4.4
)
FetchContent_MakeAvailable(ixwebsocket)

# Fetch nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# JUCE (from parent directory)
set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/JUCE")
if(EXISTS "${JUCE_DIR}/CMakeLists.txt")
    add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)
endif()

# Engine executable (headless)
add_executable(AiceCube_Engine
    src/EngineMain.cpp
    
    # IPC
    ipc/WebSocketServer.cpp
    ipc/MessageHandler.cpp
    
    # Model (shared from main project)
    ../src/model/ProjectState.cpp
)

target_include_directories(AiceCube_Engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(AiceCube_Engine PRIVATE
    ixwebsocket
    nlohmann_json::nlohmann_json
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_processors
)

target_compile_definitions(AiceCube_Engine PRIVATE
    JUCE_STANDALONE_APPLICATION=1
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
)

# Windows-specific
if(WIN32)
    target_link_libraries(AiceCube_Engine PRIVATE ws2_32)
endif()
