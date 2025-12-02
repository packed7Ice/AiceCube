#include "TimelineComponent.h"

TimelineComponent::TimelineComponent(ProjectState& state) : projectState(state)
{
    addKeyListener(this);
    setWantsKeyboardFocus(true);
    updateTimeline();
}

TimelineComponent::~TimelineComponent()
{
    removeKeyListener(this);
}

void TimelineComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    
    // Draw Grid
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    int numBeats = 100; // Arbitrary max for now
    for (int i = 0; i < numBeats; ++i)
    {
        float x = (float)beatsToX(i);
        g.drawVerticalLine((int)x, 0.0f, (float)getHeight());
    }
    
    // Draw Loop Region
    if (projectState.isLooping)
    {
        int x1 = beatsToX(projectState.loopStart);
        int x2 = beatsToX(projectState.loopEnd);
        
        g.setColour(juce::Colours::green.withAlpha(0.1f));
        g.fillRect(x1, 0, x2 - x1, getHeight());
        
        g.setColour(juce::Colours::green);
        g.drawVerticalLine(x1, 0.0f, (float)getHeight());
        g.drawVerticalLine(x2, 0.0f, (float)getHeight());
        
        // Markers
        juce::Path startMarker;
        startMarker.addTriangle((float)x1, 0.0f, (float)x1 + 10.0f, 0.0f, (float)x1, 10.0f);
        g.fillPath(startMarker);
        
        juce::Path endMarker;
        endMarker.addTriangle((float)x2, 0.0f, (float)x2 - 10.0f, 0.0f, (float)x2, 10.0f);
        g.fillPath(endMarker);
    }
    
    // Draw Track Separators and Automation
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    for (int i = 0; i < projectState.tracks.size(); ++i)
    {
        int y = i * trackHeight;
        g.drawHorizontalLine(y + trackHeight, 0.0f, (float)getWidth());
        
        // Draw Automation (Overlay)
        auto& track = projectState.tracks[i];
        for (const auto& curve : track->automationCurves)
        {
            if (!curve.active) continue;
            
            g.setColour(juce::Colours::cyan.withAlpha(0.7f));
            juce::Path path;
            bool first = true;
            
            for (const auto& point : curve.points)
            {
                float px = (float)beatsToX(point.time);
                float py = y + trackHeight - (point.value * trackHeight);
                
                if (first)
                {
                    path.startNewSubPath(px, py);
                    first = false;
                }
                else
                {
                    path.lineTo(px, py);
                }
            }
            g.strokePath(path, juce::PathStrokeType(2.0f));
            
            // Draw points
            g.setColour(juce::Colours::white);
            for (const auto& point : curve.points)
            {
                float px = (float)beatsToX(point.time);
                float py = y + trackHeight - (point.value * trackHeight);
                g.fillEllipse(px - 3, py - 3, 6, 6);
            }
        }
    }
    
    // Draw Playhead
    if (projectState.playheadBeat >= 0)
    {
        float x = (float)beatsToX(projectState.playheadBeat);
        g.setColour(juce::Colours::yellow);
        g.drawLine(x, 0.0f, x, (float)getHeight(), 2.0f);
    }
    
    // Draw Selection Rect
    if (isSelecting)
    {
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.fillRect(selectionRect);
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.drawRect(selectionRect);
    }
}

void TimelineComponent::resized()
{
    updateTimeline();
}

void TimelineComponent::mouseDown(const juce::MouseEvent& e)
{
    grabKeyboardFocus();
    
    // Right Click (Track Context Menu)
    if (e.mods.isRightButtonDown())
    {
        int trackIndex = yToTrackIndex(e.y);
        if (trackIndex >= 0 && trackIndex < projectState.tracks.size())
        {
            juce::PopupMenu m;
            m.addItem(1, "Add Clip");
            
            m.showMenuAsync(juce::PopupMenu::Options(), [this, trackIndex, e](int result) {
                if (result == 1)
                {
                    double beat = xToBeats(e.x);
                    if (snapEnabled) beat = std::round(beat / snapResolution) * snapResolution;
                    projectState.addClip(trackIndex, beat, 4.0);
                    updateTimeline();
                }
            });
        }
        return;
    }
    
    // Add Clip (Double Click)
    if (e.getNumberOfClicks() == 2 && !e.mods.isAltDown())
    {
        int trackIndex = yToTrackIndex(e.y);
        if (trackIndex >= 0 && trackIndex < projectState.tracks.size())
        {
            double beat = xToBeats(e.x);
            if (snapEnabled) beat = std::round(beat / snapResolution) * snapResolution;
            projectState.addClip(trackIndex, beat, 4.0);
            updateTimeline();
            return;
        }
    }
    
    // Loop Range Setting (Ctrl + Drag)
    if (e.mods.isCtrlDown())
    {
        isDraggingLoop = true;
        loopDragStartBeat = xToBeats(e.x);
        if (snapEnabled) loopDragStartBeat = std::round(loopDragStartBeat / snapResolution) * snapResolution;
        if (loopDragStartBeat < 0) loopDragStartBeat = 0;
        
        projectState.loopStart = loopDragStartBeat;
        projectState.loopEnd = loopDragStartBeat;
        projectState.isLooping = true;
        repaint();
        return;
    }
    
    // Automation Editing
    draggingAutomationTrackIndex = -1;
    draggingAutomationPointIndex = -1;
    
    int trackIndex = yToTrackIndex(e.y);
    if (trackIndex >= 0 && trackIndex < projectState.tracks.size())
    {
        auto& track = projectState.tracks[trackIndex];
        int trackY = trackIndex * trackHeight;
        
        // Check active automation curves
        for (auto& curve : track->automationCurves)
        {
            if (!curve.active) continue;
            
            // Hit test points
            for (int i = 0; i < curve.points.size(); ++i)
            {
                float px = (float)beatsToX(curve.points[i].time);
                float py = trackY + trackHeight - (curve.points[i].value * trackHeight);
                
                if (e.getPosition().getDistanceFrom({(int)px, (int)py}) <= automationPointRadius + 2)
                {
                    if (e.mods.isRightButtonDown())
                    {
                        // Delete point
                        curve.points.erase(curve.points.begin() + i);
                        repaint();
                        return;
                    }
                    else
                    {
                        draggingAutomationTrackIndex = trackIndex;
                        draggingAutomationPointIndex = i;
                        return; // Consume event
                    }
                }
            }
            
            // Add point if clicking near curve
            if (e.mods.isAltDown() || e.getNumberOfClicks() == 2) // Alt+Click or Double Click to add
            {
                double beat = xToBeats(e.x);
                float val = 1.0f - (float)(e.y - trackY) / trackHeight;
                val = std::max(0.0f, std::min(1.0f, val));
                
                curve.points.push_back({ beat, val });
                std::sort(curve.points.begin(), curve.points.end(), [](const AutomationPoint& a, const AutomationPoint& b) {
                    return a.time < b.time;
                });
                
                repaint();
                return;
            }
        }
    }
    
    if (e.mods.isShiftDown())
    {
        // Range Selection Start
        isSelecting = true;
        selectionRect.setPosition(e.getPosition());
        selectionRect.setSize(0, 0);
    }
    else
    {
        // Move Playhead
        double beat = xToBeats(e.x);
        if (snapEnabled) beat = std::round(beat / snapResolution) * snapResolution;
        if (beat < 0) beat = 0;
        projectState.playheadBeat = beat;
        repaint();
        
        // Deselect if clicking empty space (unless Ctrl is down)
        if (!e.mods.isCommandDown())
        {
            selectedClips.clear();
        }
    }
}

void TimelineComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (draggingAutomationTrackIndex >= 0 && draggingAutomationPointIndex >= 0)
    {
        auto& track = projectState.tracks[draggingAutomationTrackIndex];
        
        for (auto& curve : track->automationCurves)
        {
            if (!curve.active) continue;
            
            if (draggingAutomationPointIndex < curve.points.size())
            {
                double beat = xToBeats(e.x);
                if (beat < 0) beat = 0;
                
                int trackY = draggingAutomationTrackIndex * trackHeight;
                float val = 1.0f - (float)(e.y - trackY) / trackHeight;
                val = std::max(0.0f, std::min(1.0f, val));
                
                curve.points[draggingAutomationPointIndex].time = beat;
                curve.points[draggingAutomationPointIndex].value = val;
                
                repaint();
            }
            break; // Only edit one curve
        }
        return;
    }

    if (isDraggingLoop)
    {
        double currentBeat = xToBeats(e.x);
        if (snapEnabled) currentBeat = std::round(currentBeat / snapResolution) * snapResolution;
        if (currentBeat < 0) currentBeat = 0;
        
        if (currentBeat < loopDragStartBeat)
        {
            projectState.loopStart = currentBeat;
            projectState.loopEnd = loopDragStartBeat;
        }
        else
        {
            projectState.loopStart = loopDragStartBeat;
            projectState.loopEnd = currentBeat;
        }
        
        // Ensure minimum loop length
        if (projectState.loopEnd <= projectState.loopStart) projectState.loopEnd = projectState.loopStart + 0.1;
        
        repaint();
        return;
    }

    if (isSelecting)
    {
        selectionRect.setSize(e.x - selectionRect.getX(), e.y - selectionRect.getY());
        selectClipsInRect(selectionRect);
        repaint();
    }
}

void TimelineComponent::mouseUp(const juce::MouseEvent& e)
{
    if (draggingAutomationTrackIndex >= 0)
    {
        // Sort points
        auto& track = projectState.tracks[draggingAutomationTrackIndex];
        for (auto& curve : track->automationCurves)
        {
            if (curve.active)
            {
                std::sort(curve.points.begin(), curve.points.end(), [](const AutomationPoint& a, const AutomationPoint& b) {
                    return a.time < b.time;
                });
            }
        }
        draggingAutomationTrackIndex = -1;
        draggingAutomationPointIndex = -1;
        repaint();
        return;
    }
    
    if (isDraggingLoop)
    {
        isDraggingLoop = false;
        repaint();
        return;
    }

    if (isSelecting)
    {
        isSelecting = false;
        repaint();
    }
}

void TimelineComponent::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (e.mods.isCommandDown()) // Zoom
    {
        pixelsPerBeat += wheel.deltaY * 10.0;
        if (pixelsPerBeat < 10.0) pixelsPerBeat = 10.0;
        if (pixelsPerBeat > 200.0) pixelsPerBeat = 200.0;
    }
    else // Scroll
    {
        scrollX -= wheel.deltaX * 20.0;
        if (scrollX < 0) scrollX = 0;
    }
    updateTimeline();
    repaint();
}

bool TimelineComponent::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent)
{
    if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey)
    {
        deleteSelectedClips();
        return true;
    }
    else if (key.getModifiers().isCommandDown() && key.getKeyCode() == 'D')
    {
        duplicateSelectedClips();
        return true;
    }
    
    return false;
}

void TimelineComponent::selectClipsInRect(const juce::Rectangle<int>& rect)
{
    selectedClips.clear();
    
    auto normalizedRect = rect;
    if (normalizedRect.getWidth() < 0) { normalizedRect.setX(normalizedRect.getX() + normalizedRect.getWidth()); normalizedRect.setWidth(-normalizedRect.getWidth()); }
    if (normalizedRect.getHeight() < 0) { normalizedRect.setY(normalizedRect.getY() + normalizedRect.getHeight()); normalizedRect.setHeight(-normalizedRect.getHeight()); }

    for (auto* cc : clipComponents)
    {
        if (cc->getBounds().intersects(normalizedRect))
        {
            // Ideally we would highlight the clip component
        }
    }
}

void TimelineComponent::deleteSelectedClips()
{
    for (auto& track : projectState.tracks)
    {
        auto it = std::remove_if(track->clips.begin(), track->clips.end(), [this](const Clip& c) {
            for (auto* sel : selectedClips)
            {
                if (sel == &c) return true;
            }
            return false;
        });
        track->clips.erase(it, track->clips.end());
    }
    selectedClips.clear();
    updateTimeline();
}

void TimelineComponent::duplicateSelectedClips()
{
    std::vector<std::pair<int, Clip>> newClips;
    
    for (int i = 0; i < projectState.tracks.size(); ++i)
    {
        auto& track = projectState.tracks[i];
        for (const auto& clip : track->clips)
        {
            for (auto* sel : selectedClips)
            {
                if (sel == &clip)
                {
                    Clip copy = clip;
                    copy.startBeat += copy.lengthBeats;
                    newClips.push_back({i, copy});
                }
            }
        }
    }
    
    for (auto& pair : newClips)
    {
        projectState.addClip(pair.first, pair.second);
    }
    updateTimeline();
}

void TimelineComponent::splitClipAtPlayhead()
{
    // Placeholder
}

void TimelineComponent::updateTimeline()
{
    clipComponents.clear();
    
    int trackIndex = 0;
    for (auto& track : projectState.tracks)
    {
        for (auto& clip : track->clips)
        {
            auto* cc = new ClipComponent(clip, pixelsPerBeat);
            int x = beatsToX(clip.startBeat);
            int w = (int)(clip.lengthBeats * pixelsPerBeat);
            int y = trackIndex * trackHeight;
            int h = trackHeight - 2;
            
            cc->setBounds(x, y, w, h);
            cc->onClipDoubleClicked = [this](Clip& c) {
                if (onClipEditRequested) onClipEditRequested(c);
            };
            
            cc->onClipRightClicked = [this](Clip& c, const juce::MouseEvent& e) {
                // Select this clip
                selectedClips.clear();
                selectedClips.push_back(&c);
                repaint();
                
                juce::PopupMenu m;
                m.addItem(1, "Delete");
                m.addItem(2, "Duplicate");
                
                m.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
                    if (result == 1)
                    {
                        deleteSelectedClips();
                    }
                    else if (result == 2)
                    {
                        duplicateSelectedClips();
                    }
                });
            };
            
            addAndMakeVisible(cc);
            clipComponents.add(cc);
        }
        trackIndex++;
    }
    repaint();
}
