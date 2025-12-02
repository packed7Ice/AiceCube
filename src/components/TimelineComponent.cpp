#include "TimelineComponent.h"

TimelineComponent::TimelineComponent(ProjectState& state) : projectState(state)
{
    updateTimeline();
}

void TimelineComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.8f));
    
    // Draw Grid
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    double beatsInView = getWidth() / pixelsPerBeat;
    double startBeat = scrollX / pixelsPerBeat;
    
    for (int i = 0; i < beatsInView + 1; ++i)
    {
        float x = (float)((std::floor(startBeat) + i) * pixelsPerBeat - scrollX);
        if (x >= 0)
            g.drawVerticalLine((int)x, 0.0f, (float)getHeight());
    }
    
    // Draw Track Rows
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    for (int i = 0; i < projectState.tracks.size(); ++i)
    {
        g.drawHorizontalLine((i + 1) * trackHeight, 0.0f, (float)getWidth());
    }
    
    // Playhead
    float playheadX = (float)(projectState.playheadBeat * pixelsPerBeat - scrollX);
    if (playheadX >= 0 && playheadX < getWidth())
    {
        g.setColour(juce::Colours::yellow);
        g.drawVerticalLine((int)playheadX, 0.0f, (float)getHeight());
    }
}

void TimelineComponent::resized()
{
    updateTimeline();
}

void TimelineComponent::updateTimeline()
{
    clipComponents.clear();
    
    for (int i = 0; i < projectState.tracks.size(); ++i)
    {
        auto& track = projectState.tracks[i];
        for (auto& clip : track->clips)
        {
            auto* cc = new ClipComponent(clip, pixelsPerBeat);
            
            // Calculate bounds
            int x = (int)(clip.startBeat * pixelsPerBeat - scrollX);
            int w = (int)(clip.lengthBeats * pixelsPerBeat);
            int y = i * trackHeight + 2;
            int h = trackHeight - 4;
            
            cc->setBounds(x, y, w, h);
            cc->onClipModified = [this] { updateTimeline(); repaint(); };
            
            addAndMakeVisible(cc);
            clipComponents.add(cc);
        }
    }
    repaint();
}

void TimelineComponent::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isLeftButtonDown() && e.getNumberOfClicks() == 2)
    {
        // Add Clip
        int trackIndex = e.y / trackHeight;
        if (trackIndex >= 0 && trackIndex < projectState.tracks.size())
        {
            double beat = (e.x + scrollX) / pixelsPerBeat;
            Clip newClip;
            newClip.startBeat = std::floor(beat);
            newClip.lengthBeats = 4.0;
            newClip.name = "New Clip";
            
            projectState.addClip(trackIndex, newClip);
            updateTimeline();
        }
    }
}

void TimelineComponent::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (e.mods.isShiftDown())
    {
        // Vertical Zoom (Track Height) - Not implemented yet fully
    }
    else
    {
        // Horizontal Zoom
        pixelsPerBeat += wheel.deltaY * 10.0;
        if (pixelsPerBeat < 10.0) pixelsPerBeat = 10.0;
        if (pixelsPerBeat > 200.0) pixelsPerBeat = 200.0;
        updateTimeline();
    }
}
