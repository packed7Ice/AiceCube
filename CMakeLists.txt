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
    
    # Model
    src/model/MusicData.h
    src/model/ProjectState.cpp
    src/model/ProjectState.h
    
    # Engine
    src/engine/AudioEngine.cpp
    src/engine/AudioEngine.h
    
    # Components
    src/components/TransportBarComponent.cpp
    src/components/TransportBarComponent.h
    src/components/TrackHeaderListComponent.cpp
    src/components/TrackHeaderListComponent.h
    src/components/TrackHeaderComponent.cpp
    src/components/TrackHeaderComponent.h
    src/components/TimelineComponent.cpp
    src/components/TimelineComponent.h
    src/components/ClipComponent.cpp
    src/components/ClipComponent.h
    src/components/PianoRollEditorComponent.cpp
    src/components/PianoRollEditorComponent.h
    src/components/MixerComponent.cpp
    src/components/MixerComponent.h
    src/components/BrowserComponent.cpp
    src/components/BrowserComponent.h
    
    # Agent / Legacy
    src/AgentPanel.cpp
    src/AgentPanel.h
    src/AgentLogic.cpp
    src/AgentLogic.h
    src/ApiClient.cpp
    src/ApiClient.h
    src/CommandExecutor.cpp
    src/CommandExecutor.h
)

# 使用する JUCE モジュール
target_link_libraries(AiceCube PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_utils
    juce::juce_audio_processors
)
