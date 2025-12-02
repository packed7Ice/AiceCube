#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

#include "../engine/AudioEngine.h"

class SettingsComponent : public juce::Component, public juce::ListBoxModel
{
public:
    SettingsComponent(juce::AudioDeviceManager& deviceManager, AudioEngine& engine);
    ~SettingsComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // ListBoxModel
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

private:
    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    std::unique_ptr<juce::AudioDeviceSelectorComponent> selector;
    
    // Plugin Tab
    juce::Component pluginTab;
    juce::ListBox pathList;
    juce::TextButton addPathButton { "Add Path" };
    juce::TextButton removePathButton { "Remove Path" };
    juce::TextButton scanButton { "Scan Plugins" };
    juce::Label statusLabel;
    
    AudioEngine& audioEngine;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};
