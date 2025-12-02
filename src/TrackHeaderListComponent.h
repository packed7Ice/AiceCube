#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "AppState.h"

class TrackHeaderComponent : public juce::Component
{
public:
    TrackHeaderComponent(Track& track, int index);
    ~TrackHeaderComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    Track& track;
    int trackIndex;
    
    juce::Label nameLabel;
    juce::ToggleButton muteButton{ "M" };
    juce::ToggleButton soloButton{ "S" };
    juce::Slider volumeSlider;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackHeaderComponent)
};

class TrackHeaderListComponent : public juce::Component
{
public:
    TrackHeaderListComponent(AppState& state);
    ~TrackHeaderListComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void updateTrackList();

private:
    AppState& appState;
    juce::OwnedArray<TrackHeaderComponent> headers;
    juce::TextButton addTrackButton{ "+" };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackHeaderListComponent)
};
