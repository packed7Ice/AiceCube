#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class PluginWindow : public juce::DocumentWindow
{
public:
    PluginWindow(juce::AudioPluginInstance& plugin, juce::AudioProcessorEditor* editor)
        : juce::DocumentWindow(plugin.getName(),
                               juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
                               juce::DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(editor, true);
        setResizable(editor->isResizable(), false);
        setResizeLimits(100, 100, 2000, 2000);
        setVisible(true);
    }

    ~PluginWindow() override
    {
        clearContentComponent();
    }

    void closeButtonPressed() override
    {
        delete this;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginWindow)
};
