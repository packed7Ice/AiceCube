#include "LeftPaneComponent.h"

LeftPaneComponent::LeftPaneComponent(AppState& state) : appState(state)
{
}

void LeftPaneComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.1f));
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds().removeFromRight(1), 1); // Separator

    g.setColour(juce::Colours::white);
    int y = 0;
    int trackHeight = 50; // Fixed for now
    
    for (int i = 0; i < appState.tracks.size(); ++i)
    {
        auto& track = appState.tracks.getReference(i);
        auto rowBounds = juce::Rectangle<int>(0, y, getWidth(), trackHeight);
        
        // Track Name
        g.setColour(juce::Colours::white);
        g.drawText(track.name, rowBounds.removeFromLeft(getWidth() - 60).reduced(5), juce::Justification::centredLeft);
        
        // Mute Button (Simple rect for now)
        auto buttonBounds = rowBounds.removeFromRight(50);
        auto muteBounds = buttonBounds.removeFromLeft(25).reduced(2);
        auto soloBounds = buttonBounds.reduced(2);
        
        // Draw Mute
        g.setColour(track.isMuted ? juce::Colours::red : juce::Colours::darkgrey);
        g.fillRect(muteBounds);
        g.setColour(juce::Colours::white);
        g.drawText("M", muteBounds, juce::Justification::centred);
        
        // Draw Solo
        g.setColour(track.isSoloed ? juce::Colours::yellow : juce::Colours::darkgrey);
        g.fillRect(soloBounds);
        g.setColour(track.isSoloed ? juce::Colours::black : juce::Colours::white);
        g.drawText("S", soloBounds, juce::Justification::centred);

        g.setColour(juce::Colours::grey);
        g.drawHorizontalLine(y + trackHeight, 0.0f, (float)getWidth());
        y += trackHeight;
    }
}

void LeftPaneComponent::mouseDown(const juce::MouseEvent& e)
{
    int trackHeight = 50;
    int trackIndex = e.y / trackHeight;
    
    if (trackIndex >= 0 && trackIndex < appState.tracks.size())
    {
        auto& track = appState.tracks.getReference(trackIndex);
        int x = e.x;
        int width = getWidth();
        
        // Check if clicked on buttons (Right 50px)
        if (x > width - 50)
        {
            if (x < width - 25) // Mute
            {
                track.isMuted = !track.isMuted;
            }
            else // Solo
            {
                track.isSoloed = !track.isSoloed;
                // Clear other solos if needed, or allow multi-solo
            }
            repaint();
        }
    }
}

void LeftPaneComponent::resized()
{
}
