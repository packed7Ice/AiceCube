cmake_minimum_required(VERSION 3.15)

project(AiceCube VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
)

# 使用する JUCE モジュール
target_link_libraries(AiceCube PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_utils
)
