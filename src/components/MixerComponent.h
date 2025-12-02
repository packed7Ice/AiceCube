#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../model/ProjectState.h"

class AudioEngine;

class MixerComponent : public juce::Component
{
public:
    MixerComponent(ProjectState& state, AudioEngine& engine);
    ~MixerComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void updateMixer();

private:
    ProjectState& projectState;
    AudioEngine& audioEngine;
    std::vector<std::unique_ptr<juce::Component>> strips;
    juce::TextButton addBusButton { "Add Bus" };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerComponent)
};
