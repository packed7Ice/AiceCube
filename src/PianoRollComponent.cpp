#include "PianoRollComponent.h"

PianoRollComponent::PianoRollComponent()
{
    addAndMakeVisible(playButton);
    playButton.onClick = [this] { if (onPlayButtonClicked) onPlayButtonClicked(); };
}

void PianoRollComponent::setSequence(const Sequence& seq)
{
    sequence = seq;
    repaint();
}

void PianoRollComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker());

    auto bounds = getLocalBounds();
    
    // Draw Grid
    g.setColour(juce::Colours::grey.withAlpha(0.3f));
    
    // Horizontal lines (pitches)
    for (int note = minNote; note <= maxNote; ++note)
    {
        float y = bounds.getHeight() - (note - minNote + 1) * noteHeight;
        g.drawHorizontalLine((int)y, 0.0f, (float)bounds.getWidth());
    }

    // Vertical lines (beats) - assuming 4/4, 4 bars for now
    for (int i = 0; i < 16; ++i)
    {
        float x = i * pixelsPerBeat;
        g.drawVerticalLine((int)x, 0.0f, (float)bounds.getHeight());
    }

    // Draw Notes
    g.setColour(juce::Colours::orange);
    for (const auto& note : sequence.notes)
    {
        // Map pitch to Y (higher pitch = lower Y value)
        // Let's say minNote is at the bottom.
        float y = bounds.getHeight() - (note.pitch - minNote + 1) * noteHeight;
        float x = (float)(note.start * pixelsPerBeat);
        float w = (float)(note.duration * pixelsPerBeat);
        
        g.fillRect(x, y, w, noteHeight - 1.0f);
        g.drawRect(x, y, w, noteHeight - 1.0f, 1.0f);
    }
}

void PianoRollComponent::resized()
{
    playButton.setBounds(10, 10, 60, 30);
}
