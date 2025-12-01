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
    
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    
    std::function<void(Clip*)> onClipDoubleClicked;

private:
    AppState& appState;
    float pixelsPerBeat = 40.0f;
    int trackHeight = 50;
    
    // Dragging state
    int draggingClipTrackIndex = -1;
    int draggingClipIndex = -1;
    int lastMouseX = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineComponent)
};
