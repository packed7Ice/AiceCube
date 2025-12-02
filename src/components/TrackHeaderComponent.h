#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include "../model/MusicData.h"

class TrackHeaderComponent : public juce::Component
{
public:
    TrackHeaderComponent(std::shared_ptr<Track> track);
    ~TrackHeaderComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    std::function<void(Track*)> onPluginButtonClicked;

private:
    std::shared_ptr<Track> track;
    
    juce::Label nameLabel;
    juce::TextButton muteButton{ "M" };
    juce::TextButton soloButton{ "S" };
    juce::TextButton recButton{ "R" };
    juce::TextButton automationButton{ "A" };
    juce::Slider volumeSlider;
    juce::Slider panSlider;
    juce::TextButton pluginButton{ "VST" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackHeaderComponent)
};
