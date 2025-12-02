#include "BrowserComponent.h"

BrowserComponent::BrowserComponent()
{
    // Initialize File Browser
    thread.startThread();
    directoryList.reset(new juce::DirectoryContentsList(nullptr, thread));
    directoryList->setDirectory(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), true, true);
    
    fileTree.reset(new juce::FileTreeComponent(*directoryList));
    fileTree->setDragAndDropDescription("FileDrag");
    
    // Setup Tabs
    tabs.addTab("Files", juce::Colours::darkgrey, fileTree.get(), false);
    tabs.addTab("Plugins", juce::Colours::darkgrey, &pluginList, false);
    tabs.addTab("Presets", juce::Colours::darkgrey, &presetList, false);
    
    addAndMakeVisible(tabs);
}

BrowserComponent::~BrowserComponent()
{
    // Tabs hold pointers to components, but we own them (except generic lists which are members)
    // fileTree is unique_ptr, so we need to make sure tabs don't delete it.
    // We passed 'false' to addTab, so tabs won't delete them.
    
    // Stop thread
    thread.stopThread(1000);
}

void BrowserComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void BrowserComponent::resized()
{
    tabs.setBounds(getLocalBounds());
}
