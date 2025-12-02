#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class BrowserComponent : public juce::Component
{
public:
    BrowserComponent();
    ~BrowserComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::FileBrowserComponent fileBrowser;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrowserComponent)
};
