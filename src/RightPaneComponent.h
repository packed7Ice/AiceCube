#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AgentPanel.h"

class RightPaneComponent : public juce::Component
{
public:
    RightPaneComponent();
    ~RightPaneComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    AgentPanel& getAgentPanel() { return agentPanel; }

private:
    AgentPanel agentPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RightPaneComponent)
};
