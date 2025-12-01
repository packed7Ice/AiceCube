#include "PianoRollComponent.h"

PianoRollComponent::PianoRollComponent()
{
    addAndMakeVisible(backButton);
    backButton.onClick = [this] { if (onBackClicked) onBackClicked(); };
}

void PianoRollComponent::setClip(Clip* clip)
{
    currentClip = clip;
    repaint();
}

void PianoRollComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.3f));

    if (currentClip == nullptr)
    {
        g.setColour(juce::Colours::white);
        g.drawText("No Clip Selected", getLocalBounds(), juce::Justification::centred);
        return;
    }

    // Draw Grid
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    for (int i = 0; i < 100; ++i)
    {
        float x = i * pixelsPerBeat;
        g.drawVerticalLine((int)x, 0.0f, (float)getHeight());
    }
    for (int i = 0; i < 128; ++i)
    {
        float y = getHeight() - (i * pixelsPerPitch);
        g.drawHorizontalLine((int)y, 0.0f, (float)getWidth());
    }

    // Draw Notes
    g.setColour(juce::Colours::cyan);
    for (const auto& note : currentClip->notes)
    {
        float x = (float)(note.start * pixelsPerBeat);
        float y = getHeight() - ((note.pitch + 1) * pixelsPerPitch);
        float w = (float)(note.duration * pixelsPerBeat);
        float h = pixelsPerPitch - 1;
        
        g.fillRect(x, y, w, h);
        g.setColour(juce::Colours::black);
        g.drawRect(x, y, w, h);
        g.setColour(juce::Colours::cyan);
    }
}

void PianoRollComponent::mouseDown(const juce::MouseEvent& e)
{
    if (currentClip == nullptr) return;

    if (e.mods.isLeftButtonDown())
    {
        // Add Note
        int pitch = getPitchAtY(e.y);
        double start = getBeatAtX(e.x);
        
        // Simple quantization to 0.25 (16th note)
        start = std::round(start * 4.0) / 4.0;
        
        Note newNote;
        newNote.pitch = pitch;
        newNote.start = start;
        newNote.duration = 1.0; // Default duration
        newNote.velocity = 100;
        
        currentClip->notes.add(newNote);
        repaint();
    }
    else if (e.mods.isRightButtonDown())
    {
        // Remove Note (Simple hit test)
        int pitch = getPitchAtY(e.y);
        double beat = getBeatAtX(e.x);
        
        for (int i = 0; i < currentClip->notes.size(); ++i)
        {
            const auto& note = currentClip->notes.getReference(i);
            if (note.pitch == pitch && beat >= note.start && beat < (note.start + note.duration))
            {
                currentClip->notes.remove(i);
                repaint();
                break;
            }
        }
    }
}

int PianoRollComponent::getPitchAtY(int y)
{
    return (int)((getHeight() - y) / pixelsPerPitch);
}

double PianoRollComponent::getBeatAtX(int x)
{
    return x / pixelsPerBeat;
}

void PianoRollComponent::resized()
{
    backButton.setBounds(10, 10, 120, 30);
}
