#include "TimelineComponent.h"

TimelineComponent::TimelineComponent(AppState& state) : appState(state)
{
}

void TimelineComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);

    // Draw Grid (Vertical lines for beats)
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    for (int i = 0; i < 100; ++i) // Draw 100 beats for now
    {
        float x = i * pixelsPerBeat;
        g.drawVerticalLine((int)x, 0.0f, (float)getHeight());
    }

    // Draw Tracks and Clips
    int y = 0;
    for (const auto& track : appState.tracks)
    {
        // Draw Track Background (Alternating?)
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.drawRect(0, y, getWidth(), trackHeight);

        // Draw Clips
        g.setColour(juce::Colours::orange.withAlpha(0.8f));
        for (const auto& clip : track.clips)
        {
            float x = (float)(clip.startBeat * pixelsPerBeat);
            float w = (float)(clip.lengthBeats * pixelsPerBeat);
            g.fillRect(x, (float)y + 2, w, (float)trackHeight - 4);
            g.setColour(juce::Colours::black);
            g.drawRect(x, (float)y + 2, w, (float)trackHeight - 4);
            g.setColour(juce::Colours::orange.withAlpha(0.8f));
        }

        y += trackHeight;
    }

    // Draw Playhead
    float playheadX = (float)(appState.playheadBeat * pixelsPerBeat);
    g.setColour(juce::Colours::yellow);
    g.drawVerticalLine((int)playheadX, 0.0f, (float)getHeight());
}

void TimelineComponent::resized()
{
}
