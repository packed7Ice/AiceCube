#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AgentPanel.h"

class BottomPaneComponent : public juce::Component
{
public:
    BottomPaneComponent();
    ~BottomPaneComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    AgentPanel& getAgentPanel() { return agentPanel; }

private:
    AgentPanel agentPanel;
    // Placeholder for Editor (right side)
    juce::Component editorPlaceholder;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BottomPaneComponent)
};
