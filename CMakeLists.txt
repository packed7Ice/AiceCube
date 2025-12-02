cmake_minimum_required(VERSION 3.15)

project(AiceCube VERSION 0.1.0)

    set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    add_compile_options(/utf-8)
endif()

# JUCE の CMake モジュールを追加
add_subdirectory(external/JUCE)

# GUI アプリケーションターゲットを追加
juce_add_gui_app(AiceCube
    PRODUCT_NAME "AiceCube"
)

# ソースファイルを登録（フォルダは src で統一）
target_sources(AiceCube PRIVATE
    src/Main.cpp
    src/MainComponent.cpp
    src/MainComponent.h
    src/AgentPanel.cpp
    src/AgentPanel.h
    src/AgentLogic.cpp
    src/AgentLogic.h
    src/ApiClient.cpp
    src/ApiClient.h
    src/PianoRollComponent.cpp
    src/PianoRollComponent.h
    src/SimpleSynth.h
    src/AppState.cpp
    src/AppState.h
    src/TransportBarComponent.cpp
    src/TransportBarComponent.h
    src/LeftPaneComponent.cpp
    src/LeftPaneComponent.h
    src/TimelineComponent.cpp
    src/TimelineComponent.h
    src/RightPaneComponent.cpp
    src/RightPaneComponent.h
    src/CommandExecutor.cpp
    src/CommandExecutor.h
    src/AudioEngine.cpp
    src/AudioEngine.h
    src/TrackHeaderListComponent.cpp
    src/TrackHeaderListComponent.h
)

# 使用する JUCE モジュール
target_link_libraries(AiceCube PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_utils
    juce::juce_audio_processors
)
