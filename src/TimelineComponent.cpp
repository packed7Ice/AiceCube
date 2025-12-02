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
        g.setColour(juce::Colours::grey.withAlpha(0.5f));
        g.drawHorizontalLine(y + trackHeight, 0.0f, (float)getWidth());

        // Draw Clips
        for (const auto& clip : track.clips)
        {
            float x = (float)(clip.startBeat * pixelsPerBeat);
            float w = (float)(clip.lengthBeats * pixelsPerBeat);
            
            if (clip.type == ClipType::Audio)
                g.setColour(juce::Colours::lightblue.withAlpha(0.8f));
            else
                g.setColour(juce::Colours::orange.withAlpha(0.8f));
                
            g.fillRect(x, (float)y + 2, w, (float)trackHeight - 4);
            
            g.setColour(juce::Colours::black);
            g.drawRect(x, (float)y + 2, w, (float)trackHeight - 4);
            
            g.setColour(juce::Colours::black);
            g.drawText(clip.name, (int)x + 2, y + 2, (int)w - 4, trackHeight - 4, juce::Justification::centredLeft, true);
        }

        y += trackHeight;
    }

    // Draw Playhead
    float playheadX = (float)(appState.playheadBeat * pixelsPerBeat);
    g.setColour(juce::Colours::yellow);
    g.drawVerticalLine((int)playheadX, 0.0f, (float)getHeight());
}

void TimelineComponent::mouseDown(const juce::MouseEvent& e)
{
    // Simple hit test
    int trackIndex = e.y / trackHeight;
    if (trackIndex >= 0 && trackIndex < appState.tracks.size())
    {
        auto& track = appState.tracks.getReference(trackIndex);
        double beat = e.x / pixelsPerBeat;
        
        // Check clips
        for (int i = 0; i < track.clips.size(); ++i)
        {
            auto& clip = track.clips.getReference(i);
            if (beat >= clip.startBeat && beat < clip.getEndBeat())
            {
                draggingClipTrackIndex = trackIndex;
                draggingClipIndex = i;
                lastMouseX = e.x;
                
                if (e.mods.isLeftButtonDown() && e.getNumberOfClicks() == 2)
                {
                    if (onClipDoubleClicked) onClipDoubleClicked(&clip);
                }
                
                return; // Found clip
            }
        }
        
        // Double click on empty space -> Add Clip
        if (e.mods.isLeftButtonDown() && e.getNumberOfClicks() == 2)
        {
            Clip newClip;
            newClip.startBeat = std::floor(beat); // Snap to beat
            newClip.lengthBeats = 4.0;
            newClip.type = (track.type == TrackType::Audio) ? ClipType::Audio : ClipType::Midi;
            newClip.name = (newClip.type == ClipType::Audio) ? "Audio Clip" : "Midi Clip";
            
            appState.addClip(trackIndex, newClip);
            repaint();
        }
    }
    
    // Deselect or other actions
    draggingClipTrackIndex = -1;
    draggingClipIndex = -1;
}

void TimelineComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (draggingClipTrackIndex != -1 && draggingClipIndex != -1)
    {
        auto& track = appState.tracks.getReference(draggingClipTrackIndex);
        if (draggingClipIndex < track.clips.size())
        {
            auto& clip = track.clips.getReference(draggingClipIndex);
            
            float deltaPx = (float)(e.x - lastMouseX);
            double deltaBeats = deltaPx / pixelsPerBeat;
            
            clip.startBeat += deltaBeats;
            if (clip.startBeat < 0) clip.startBeat = 0;
            
            lastMouseX = e.x;
            repaint();
        }
    }
}

void TimelineComponent::mouseUp(const juce::MouseEvent& e)
{
    draggingClipTrackIndex = -1;
    draggingClipIndex = -1;
}

void TimelineComponent::resized()
{
}
