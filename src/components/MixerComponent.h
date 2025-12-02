#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../model/ProjectState.h"

class MixerComponent : public juce::Component
{
public:
    MixerComponent(ProjectState& state);
    ~MixerComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    ProjectState& projectState;
    // TODO: Add channel strips
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerComponent)
};
