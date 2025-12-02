#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AppState.h"
#include "TrackHeaderListComponent.h"

class LeftPaneComponent : public juce::Component
{
public:
    LeftPaneComponent(AppState& state);
    ~LeftPaneComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    AppState& appState;
    TrackHeaderListComponent trackHeaders;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeftPaneComponent)
};
