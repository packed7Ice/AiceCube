#include "BrowserComponent.h"

BrowserComponent::BrowserComponent()
{
    // Initialize File Browser
    thread.startThread();
    directoryList.reset(new juce::DirectoryContentsList(nullptr, thread));
    directoryList->setDirectory(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), true, true);
    
    fileTree.reset(new juce::FileTreeComponent(*directoryList));
    fileTree->setDragAndDropDescription("FileDrag");
    
    addAndMakeVisible(fileTree.get());
}

BrowserComponent::~BrowserComponent()
{
    thread.stopThread(1000);
}

void BrowserComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void BrowserComponent::resized()
{
    if (fileTree)
        fileTree->setBounds(getLocalBounds());
}
