#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AppState.h"

class TransportBarComponent : public juce::Component
{
public:
    TransportBarComponent(AppState& state);
    ~TransportBarComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void()> onPlayClicked;
    std::function<void()> onStopClicked;

private:
    AppState& appState;
    juce::TextButton playButton{ "Play" };
    juce::TextButton stopButton{ "Stop" };
    juce::Label tempoLabel;
    juce::Label positionLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBarComponent)
};
