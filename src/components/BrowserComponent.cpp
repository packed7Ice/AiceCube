#include "BrowserComponent.h"

BrowserComponent::BrowserComponent()
    : fileBrowser(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                  juce::File(), nullptr, nullptr)
{
    addAndMakeVisible(fileBrowser);
}

void BrowserComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.1f));
}

void BrowserComponent::resized()
{
    fileBrowser.setBounds(getLocalBounds());
}
