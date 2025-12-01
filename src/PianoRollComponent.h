#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "MusicData.h"

class PianoRollComponent : public juce::Component
{
public:
    PianoRollComponent();
    ~PianoRollComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setSequence(const Sequence& seq);
    const Sequence& getSequence() const { return sequence; }

    // Playback control
    std::function<void()> onPlayButtonClicked;

private:
    Sequence sequence;
    juce::TextButton playButton{ "Play" };

    // Drawing helpers
    float noteHeight = 10.0f;
    float pixelsPerBeat = 40.0f;
    int minNote = 36; // C2
    int maxNote = 84; // C6

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollComponent)
};
