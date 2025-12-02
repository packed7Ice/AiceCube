#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../model/MusicData.h"

class TrackHeaderComponent : public juce::Component
{
public:
    TrackHeaderComponent(std::shared_ptr<Track> track, int index);
    ~TrackHeaderComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    std::shared_ptr<Track> track;
    int trackIndex;

    juce::Label nameLabel;
    juce::TextButton muteButton{ "M" };
    juce::TextButton soloButton{ "S" };
    juce::Slider volumeSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackHeaderComponent)
};
