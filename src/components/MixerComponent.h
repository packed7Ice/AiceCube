#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../model/ProjectState.h"

class MixerChannelStrip;

class MixerComponent : public juce::Component
{
public:
    MixerComponent(ProjectState& state);
    ~MixerComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void updateMixer();

private:
    ProjectState& projectState;
    juce::OwnedArray<MixerChannelStrip> strips;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerComponent)
};
