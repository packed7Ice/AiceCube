#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../model/ProjectState.h"
#include "ClipComponent.h"

class TimelineComponent : public juce::Component
{
public:
    TimelineComponent(ProjectState& state);
    ~TimelineComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    void updateTimeline();

private:
    ProjectState& projectState;
    juce::OwnedArray<ClipComponent> clipComponents;
    
    double pixelsPerBeat = 40.0;
    int trackHeight = 60;
    double scrollX = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineComponent)
};
