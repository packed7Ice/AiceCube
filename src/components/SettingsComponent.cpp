#include "SettingsComponent.h"

#include "SettingsComponent.h"

SettingsComponent::SettingsComponent(juce::AudioDeviceManager& deviceManager, AudioEngine& engine)
    : audioEngine(engine)
{
    setSize(600, 500);
    
    // Audio Tab
    selector.reset(new juce::AudioDeviceSelectorComponent(deviceManager, 0, 256, 0, 256, true, true, true, false));
    tabs.addTab("Audio Device", juce::Colours::darkgrey, selector.get(), false);
    
    // Plugin Tab
    pathList.setModel(this);
    pathList.setColour(juce::ListBox::backgroundColourId, juce::Colours::black.withAlpha(0.5f));
    
    pluginTab.addAndMakeVisible(pathList);
    pluginTab.addAndMakeVisible(addPathButton);
    pluginTab.addAndMakeVisible(removePathButton);
    pluginTab.addAndMakeVisible(scanButton);
    pluginTab.addAndMakeVisible(statusLabel);
    
    addPathButton.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser>("Select VST3 Folder", juce::File::getSpecialLocation(juce::File::userHomeDirectory));
        auto flags = juce::FileBrowserComponent::canSelectDirectories | juce::FileBrowserComponent::openMode;
        fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc) {
            auto dir = fc.getResult();
            if (dir.exists()) {
                audioEngine.addPluginSearchPath(dir);
                pathList.updateContent();
            }
        });
    };
    
    removePathButton.onClick = [this] {
        int row = pathList.getSelectedRow();
        if (row >= 0) {
            audioEngine.removePluginSearchPath(row);
            pathList.updateContent();
        }
    };
    
    scanButton.onClick = [this] {
        scanButton.setEnabled(false);
        statusLabel.setText("Scanning...", juce::dontSendNotification);
        audioEngine.scanPluginsAsync(
            [this](const juce::String& name) { statusLabel.setText("Scanning: " + name, juce::dontSendNotification); },
            [this] { 
                statusLabel.setText("Scan Finished", juce::dontSendNotification); 
                scanButton.setEnabled(true);
            }
        );
    };
    
    tabs.addTab("Plugins", juce::Colours::darkgrey, &pluginTab, false);
    addAndMakeVisible(tabs);
}

SettingsComponent::~SettingsComponent()
{
    pathList.setModel(nullptr);
}

void SettingsComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void SettingsComponent::resized()
{
    tabs.setBounds(getLocalBounds());
    
    // Layout Plugin Tab
    auto area = pluginTab.getLocalBounds().reduced(10);
    auto topRow = area.removeFromTop(30);
    addPathButton.setBounds(topRow.removeFromLeft(100));
    topRow.removeFromLeft(10);
    removePathButton.setBounds(topRow.removeFromLeft(100));
    topRow.removeFromLeft(10);
    scanButton.setBounds(topRow.removeFromLeft(100));
    
    area.removeFromTop(10);
    statusLabel.setBounds(area.removeFromBottom(20));
    pathList.setBounds(area);
}

int SettingsComponent::getNumRows()
{
    return (int)audioEngine.getPluginSearchPaths().size();
}

void SettingsComponent::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::blue.withAlpha(0.5f));
        
    g.setColour(juce::Colours::white);
    if (rowNumber < audioEngine.getPluginSearchPaths().size())
    {
        g.drawText(audioEngine.getPluginSearchPaths()[rowNumber].getFullPathName(), 5, 0, width - 10, height, juce::Justification::centredLeft);
    }
}
