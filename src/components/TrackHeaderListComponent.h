#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../model/ProjectState.h"
#include "../engine/AudioEngine.h"
#include "TrackHeaderComponent.h"

class TrackHeaderListComponent : public juce::Component
{
public:
    TrackHeaderListComponent(ProjectState& state, AudioEngine& engine);
    ~TrackHeaderListComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void updateTrackList();

private:
    ProjectState& projectState;
    AudioEngine& audioEngine;
    juce::OwnedArray<TrackHeaderComponent> headers;
    juce::TextButton addTrackButton{ "+" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackHeaderListComponent)
};
