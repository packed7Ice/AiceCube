#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AppState.h"

class LeftPaneComponent : public juce::Component
{
public:
    LeftPaneComponent(AppState& state);
    ~LeftPaneComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    AppState& appState;
    // In future, this will hold a ListBox or similar
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeftPaneComponent)
};
