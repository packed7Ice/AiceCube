#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class BrowserComponent : public juce::Component, 
                         public juce::DragAndDropContainer
{
public:
    BrowserComponent();
    ~BrowserComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // File Browser
    juce::TimeSliceThread thread { "FileBrowserThread" };
    std::unique_ptr<juce::DirectoryContentsList> directoryList;
    std::unique_ptr<juce::FileTreeComponent> fileTree;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrowserComponent)
};
