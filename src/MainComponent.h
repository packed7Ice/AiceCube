#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/ProjectState.h"
#include "engine/AudioEngine.h"
#include "components/TransportBarComponent.h"
#include "components/TrackHeaderListComponent.h"
#include "components/TimelineComponent.h"
#include "components/MixerComponent.h"
#include "components/BrowserComponent.h"
#include "components/PianoRollEditorComponent.h"
#include "components/ModernLookAndFeel.h"
#include "AgentPanel.h"
#include "CommandExecutor.h"
#include "AgentLogic.h"

class MainComponent : public juce::AudioAppComponent, public juce::DragAndDropContainer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // File Operations
    void saveProject();
    void loadProject();
    void importMidi();
    void exportMidi();

private:
    // Model & Engine
    ProjectState projectState;
    AudioEngine audioEngine;

    // Agent
    ApiClient apiClient;
    CommandExecutor commandExecutor;

    // UI Components
    TransportBarComponent transportBar;
    TrackHeaderListComponent trackHeaders;
    TimelineComponent timeline;
    MixerComponent mixer;
    PianoRollEditorComponent pianoRoll;
    bool showPianoRoll = false;
    
    juce::Component::SafePointer<juce::DocumentWindow> fileBrowserWindow;
    
    AgentPanel agentPanel;
    
    ModernLookAndFeel modernLookAndFeel;
    
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
