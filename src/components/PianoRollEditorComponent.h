#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../model/MusicData.h"

class PianoRollEditorComponent : public juce::Component
{
public:
    PianoRollEditorComponent(Clip& clip);
    ~PianoRollEditorComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    Clip& clip;
    double pixelsPerBeat = 100.0;
    int noteHeight = 20;
    int scrollX = 0;
    int scrollY = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollEditorComponent)
};
