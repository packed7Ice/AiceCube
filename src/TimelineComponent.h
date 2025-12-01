#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AppState.h"

class TimelineComponent : public juce::Component
{
public:
    TimelineComponent(AppState& state);
    ~TimelineComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    AppState& appState;
    float pixelsPerBeat = 40.0f;
    int trackHeight = 50;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineComponent)
};
