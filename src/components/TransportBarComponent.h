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
    std::function<void()> onImportAudioClicked;

private:
    ProjectState& projectState;
    
    juce::TextButton playButton{ "Play" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton importButton{ "Import" };
    juce::Label tempoLabel;
    juce::Slider tempoSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBarComponent)
};
