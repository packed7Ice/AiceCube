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
    void mouseDown(const juce::MouseEvent& e) override;
    
    void setSelected(bool selected);
    std::function<void()> onSelect;
    std::function<void(Track*)> onPluginButtonClicked;
    std::function<void(Track*)> onDeleteTrack;



private:
    std::shared_ptr<Track> track;
    bool selected = false;
    
    juce::Label nameLabel;
    juce::TextButton muteButton{ "M" };
    juce::TextButton soloButton{ "S" };
    juce::TextButton recButton{ "R" };
    juce::TextButton automationButton{ "A" };
    juce::TextButton pluginButton{ "VST" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackHeaderComponent)
};
