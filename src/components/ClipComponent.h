#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../model/MusicData.h"

class ClipComponent : public juce::Component
{
public:
    ClipComponent(Clip& clip, double pixelsPerBeat);
    ~ClipComponent() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;

    std::function<void()> onClipModified;
    std::function<void()> onClipDoubleClicked;

private:
    Clip& clip;
    double pixelsPerBeat;
    
    bool isResizing = false;
    double originalStartBeat = 0.0;
    double originalLength = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipComponent)
};
