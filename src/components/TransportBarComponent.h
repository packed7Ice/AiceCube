#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../model/ProjectState.h"

class TransportBarComponent : public juce::Component
{
public:
    TransportBarComponent(ProjectState& state);
    ~TransportBarComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void()> onPlayClicked;
    std::function<void()> onStopClicked;
    std::function<void()> onRecordClicked;
    std::function<void()> onImportAudioClicked;
    std::function<void()> onSaveClicked;
    std::function<void()> onLoadClicked;
    std::function<void()> onSettingsClicked;
    std::function<void()> onFilesClicked;

private:
    ProjectState& projectState;
    
    juce::TextButton playButton{ "Play" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton recordButton{ "Record" };
    juce::TextButton importButton{ "Import" };
    juce::TextButton saveButton{ "Save" };
    juce::TextButton loadButton{ "Load" };
    juce::TextButton settingsButton{ "Settings" };
    juce::TextButton filesButton{ "Files" };
    juce::TextButton metronomeButton{ "Metronome" };
    juce::TextButton loopButton{ "Loop" };
    juce::Label tempoLabel;
    juce::Slider tempoSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBarComponent)
};
