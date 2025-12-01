#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AppState.h"

class PianoRollComponent : public juce::Component
{
public:
    PianoRollComponent();
    ~PianoRollComponent() override = default;

    void setClip(Clip* clip);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& e) override;

    std::function<void()> onBackClicked;

private:
    Clip* currentClip = nullptr;
    juce::TextButton backButton{ "Back to Timeline" };
    
    float pixelsPerBeat = 40.0f;
    float pixelsPerPitch = 15.0f;
    
    int getPitchAtY(int y);
    double getBeatAtX(int x);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollComponent)
};
