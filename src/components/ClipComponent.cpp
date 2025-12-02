#include "ClipComponent.h"

ClipComponent::ClipComponent(Clip& c, double ppb)
    : clip(c), pixelsPerBeat(ppb)
{
}

void ClipComponent::paint(juce::Graphics& g)
{
    g.fillAll(clip.clipColor.withAlpha(0.8f));
    g.setColour(juce::Colours::black);
    g.drawRect(getLocalBounds(), 1);
    g.drawText(clip.name, getLocalBounds().reduced(2), juce::Justification::centred, true);
    
    // Resize handles
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.fillRect(getWidth() - 5, 0, 5, getHeight());
}

void ClipComponent::mouseDown(const juce::MouseEvent& e)
{
    originalStartBeat = clip.startBeat;
    originalLength = clip.lengthBeats;
    
    if (e.getNumberOfClicks() == 2)
    {
        if (onClipDoubleClicked) onClipDoubleClicked();
        return;
    }

    if (e.x > getWidth() - 10)
        isResizing = true;
    else
        isResizing = false;
}

void ClipComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (isResizing)
    {
        double diffBeats = (e.getPosition().x - e.getMouseDownX()) / pixelsPerBeat;
        double newLength = originalLength + diffBeats;
        if (newLength < 0.25) newLength = 0.25;
        clip.lengthBeats = newLength;
        
        setSize((int)(newLength * pixelsPerBeat), getHeight());
    }
    else
    {
        double diffBeats = (e.getDistanceFromDragStartX()) / pixelsPerBeat;
        double newStart = originalStartBeat + diffBeats;
        if (newStart < 0) newStart = 0;
        clip.startBeat = newStart;
        
        setTopLeftPosition((int)(newStart * pixelsPerBeat), getY());
    }
    
    if (onClipModified) onClipModified();
}
