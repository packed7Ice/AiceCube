#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../model/MusicData.h"
#include "../model/ProjectState.h"

class PianoRollEditorComponent : public juce::Component, public juce::Timer
{
public:
    PianoRollEditorComponent(ProjectState& state) : projectState(state)
    {
        setSize(800, 600);
        
        addAndMakeVisible(gridSelect);
        gridSelect.addItem("1/4 Beat", 1);
        gridSelect.addItem("1/8 Beat", 2);
        gridSelect.addItem("1/16 Beat", 3);
        gridSelect.addItem("1/32 Beat", 4);
        gridSelect.setSelectedId(3); // Default 1/16
        gridSelect.onChange = [this] {
            switch (gridSelect.getSelectedId())
            {
                case 1: snapResolution = 1.0; break;
                case 2: snapResolution = 0.5; break;
                case 3: snapResolution = 0.25; break;
                case 4: snapResolution = 0.125; break;
            }
            repaint();
        };
        
        startTimer(30); // For playback animation
        
        // Set initial scroll to C5 (Note 72)
        // (127 - 72) * 20 = 1100. Center roughly at 800.
        scrollY = 800;
    }
    
    void setClip(Clip* c)
    {
        clip = c;
        repaint();
    }
    
    ~PianoRollEditorComponent() override { stopTimer(); }
    
    void timerCallback() 
    {
        if (projectState.isPlaying && isVisible())
        {
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey.darker(0.5f));
        
        if (!clip)
        {
            g.setColour(juce::Colours::white);
            g.drawText("No Clip Selected", getLocalBounds(), juce::Justification::centred, true);
            return;
        }
        
        auto bounds = getLocalBounds();
        auto topBar = bounds.removeFromTop(30);
        
        // Draw Top Bar Background
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillRect(topBar);
        
        auto velocityArea = bounds.removeFromBottom(velocityHeight);
        auto keyboardArea = bounds.removeFromLeft(keyboardWidth);
        auto contentArea = bounds;
        
        // 1. Draw Keyboard
        {
            juce::Graphics::ScopedSaveState save(g);
            g.reduceClipRegion(0, topBar.getBottom(), keyboardWidth, bounds.getHeight());
            
            g.setColour(juce::Colours::black);
            g.fillRect(keyboardArea);
            
            for (int i = 0; i < 128; ++i)
            {
                int pitch = 127 - i;
                int y = pitchToY(pitch) + topBar.getBottom();
                int h = noteHeight;
                
                // Optimization: Skip if out of view
                if (y + h < topBar.getBottom() || y > getHeight() - velocityHeight) continue;
                
                bool isBlack = (pitch % 12 == 1 || pitch % 12 == 3 || pitch % 12 == 6 || pitch % 12 == 8 || pitch % 12 == 10);
                
                g.setColour(isBlack ? juce::Colours::black : juce::Colours::white);
                g.fillRect(0, y, keyboardWidth, h);
                g.setColour(juce::Colours::grey);
                g.drawRect(0, y, keyboardWidth, h, 1);
                
                if (!isBlack && pitch % 12 == 0) // C
                {
                    g.setColour(juce::Colours::black);
                    g.drawText("C" + juce::String(pitch / 12 - 1), 5, y, keyboardWidth - 10, h, juce::Justification::centredRight);
                }
                else if (!isBlack)
                {
                     juce::String noteName;
                     int p = pitch % 12;
                     if (p == 2) noteName = "D";
                     else if (p == 4) noteName = "E";
                     else if (p == 5) noteName = "F";
                     else if (p == 7) noteName = "G";
                     else if (p == 9) noteName = "A";
                     else if (p == 11) noteName = "B";
                     
                     if (noteHeight > 12) // Only draw text if height is enough
                     {
                         g.setColour(juce::Colours::black);
                         g.setFont(juce::jmin(10.0f, (float)noteHeight - 2));
                         g.drawText(noteName, 5, y, keyboardWidth - 20, h, juce::Justification::centredRight);
                     }
                }
            }
        }
        
        // 2. Draw Content (Grid & Notes)
        {
            juce::Graphics::ScopedSaveState save(g);
            g.reduceClipRegion(contentArea);
            
            // Background
            g.setColour(juce::Colours::darkgrey.darker(0.8f));
            g.fillRect(contentArea);

            // Grid Lines
            // Vertical (Beats)
            double startBeat = xToBeats(contentArea.getX());
            double endBeat = xToBeats(contentArea.getRight());
            
            // Draw beats
            g.setColour(juce::Colours::white.withAlpha(0.1f));
            for (double b = std::floor(startBeat); b <= endBeat; b += 1.0)
            {
                int x = beatsToX(b);
                g.drawVerticalLine(x, (float)contentArea.getY(), (float)contentArea.getBottom());
            }
            
            // Draw sub-beats (based on snap)
            if (pixelsPerBeat * snapResolution > 5.0)
            {
                g.setColour(juce::Colours::white.withAlpha(0.05f));
                for (double b = std::floor(startBeat); b <= endBeat; b += snapResolution)
                {
                    int x = beatsToX(b);
                    g.drawVerticalLine(x, (float)contentArea.getY(), (float)contentArea.getBottom());
                }
            }

            // Horizontal (Pitch)
            for (int i = 0; i < 128; ++i)
            {
                int pitch = 127 - i;
                int y = pitchToY(pitch) + topBar.getBottom();
                if (y + noteHeight < topBar.getBottom() || y > contentArea.getBottom()) continue;
                
                bool isBlack = (pitch % 12 == 1 || pitch % 12 == 3 || pitch % 12 == 6 || pitch % 12 == 8 || pitch % 12 == 10);
                
                g.setColour(isBlack ? juce::Colours::black.withAlpha(0.2f) : juce::Colours::white.withAlpha(0.05f));
                g.fillRect(contentArea.getX(), y, contentArea.getWidth(), noteHeight);
                
                g.setColour(juce::Colours::white.withAlpha(0.1f));
                g.drawHorizontalLine(y, (float)contentArea.getX(), (float)contentArea.getRight());
            }
            
            // Notes
            for (int i = 0; i < clip->midiSequence.getNumEvents(); ++i)
            {
                auto* event = clip->midiSequence.getEventPointer(i);
                if (event->message.isNoteOn())
                {
                    int note = event->message.getNoteNumber();
                    double noteStart = event->message.getTimeStamp();
                    
                    auto* noteOff = clip->midiSequence.getEventPointer(clip->midiSequence.getIndexOfMatchingKeyUp(i));
                    double noteEnd = noteOff ? noteOff->message.getTimeStamp() : (noteStart + 1.0);
                    
                    double duration = noteEnd - noteStart;
                    
                    int x = beatsToX(noteStart);
                    int y = pitchToY(note) + topBar.getBottom();
                    int w = (int)(duration * pixelsPerBeat);
                    int h = noteHeight;
                    
                    if (x + w < contentArea.getX() || x > contentArea.getRight()) continue;
                    if (y + h < contentArea.getY() || y > contentArea.getBottom()) continue;
                    
                    if (i == selectedNoteIndex)
                        g.setColour(juce::Colours::orange.brighter());
                    else
                        g.setColour(juce::Colours::orange);
                    
                    g.fillRect(x, y, w, h - 1);
                    
                    g.setColour(juce::Colours::black);
                    g.drawRect(x, y, w, h - 1);
                }
            }
            
            // Playhead
            if (projectState.isPlaying)
            {
                // Calculate playhead position relative to clip start
                // Global playheadBeat is absolute. Clip has startBeat.
                // But piano roll shows relative to clip? Or absolute?
                // Usually piano roll shows the clip contents.
                // If clip starts at 4.0, and playhead is at 5.0, then inside clip it is 1.0.
                // But our xToBeats/beatsToX seems to be 0-based relative to clip start?
                // Let's assume midi events are relative to clip start (0.0).
                
                double relBeat = projectState.playheadBeat - clip->startBeat;
                if (relBeat >= 0 && relBeat <= clip->lengthBeats)
                {
                    int x = beatsToX(relBeat);
                    g.setColour(juce::Colours::yellow);
                    g.drawVerticalLine(x, (float)contentArea.getY(), (float)contentArea.getBottom());
                    
                    // Triangle head
                    juce::Path head;
                    head.addTriangle((float)x - 5, (float)contentArea.getY(), 
                                     (float)x + 5, (float)contentArea.getY(), 
                                     (float)x, (float)contentArea.getY() + 10);
                    g.fillPath(head);
                }
            }
        }
        
        // 3. Draw Velocity Lane
        {
            juce::Graphics::ScopedSaveState save(g);
            g.reduceClipRegion(velocityArea);
            
            g.setColour(juce::Colours::black);
            g.fillRect(velocityArea);
            g.setColour(juce::Colours::white);
            g.drawHorizontalLine(velocityArea.getY(), 0.0f, (float)getWidth());
            
            for (int i = 0; i < clip->midiSequence.getNumEvents(); ++i)
            {
                auto* event = clip->midiSequence.getEventPointer(i);
                if (event->message.isNoteOn())
                {
                    double noteStart = event->message.getTimeStamp();
                    int x = beatsToX(noteStart);
                    float vel = event->message.getVelocity() / 127.0f;
                    float h = vel * (velocityHeight - 5); // Margin
                    float y = (float)velocityArea.getBottom() - h;
                    
                    if (x < keyboardWidth) continue; // Skip if under keyboard
                    
                    if (i == selectedNoteIndex)
                        g.setColour(juce::Colours::orange.brighter());
                    else
                        g.setColour(juce::Colours::orange);
                        
                    g.fillRect((float)x, y, 5.0f, h);
                }
            }
        }
    }
    
    void resized() override 
    {
        gridSelect.setBounds(10, 5, 100, 20);
    }
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (!clip) return;
        
        int topOffset = 30;
        
        if (e.y < topOffset) return; // Top bar

        if (e.y > getHeight() - velocityHeight)
        {
            // Velocity Edit
            isEditingVelocity = true;
            editVelocity(e);
            return;
        }

        if (e.x < keyboardWidth) return;
        
        int noteIdx = getNoteAt(e.x, e.y);
        
        if (noteIdx != -1)
        {
            selectedNoteIndex = noteIdx;
            isDragging = true;
            
            auto* event = clip->midiSequence.getEventPointer(noteIdx);
            originalNoteStart = event->message.getTimeStamp();
            originalNotePitch = event->message.getNoteNumber();
            
            dragStartBeat = xToBeats(e.x);
            dragStartPitch = yToPitch(e.y - topOffset);
            
            double noteEnd = originalNoteStart + 1.0;
            auto* noteOff = clip->midiSequence.getEventPointer(clip->midiSequence.getIndexOfMatchingKeyUp(noteIdx));
            if (noteOff) noteEnd = noteOff->message.getTimeStamp();
            
            int noteRightX = beatsToX(noteEnd);
            if (e.x >= noteRightX - 5)
            {
                isResizing = true;
                originalNoteDuration = noteEnd - originalNoteStart;
            }
            else
            {
                isResizing = false;
            }
        }
        else
        {
            selectedNoteIndex = -1;
            if (e.getNumberOfClicks() == 2)
            {
                double beat = xToBeats(e.x);
                int note = yToPitch(e.y - topOffset);
                
                if (note >= 0 && note < 128)
                {
                    // Snap
                    if (!e.mods.isAltDown())
                        beat = std::round(beat / snapResolution) * snapResolution;
                    
                    auto m = juce::MidiMessage::noteOn(1, note, (juce::uint8)100);
                    m.setTimeStamp(beat);
                    clip->midiSequence.addEvent(m);
                    
                    auto off = juce::MidiMessage::noteOff(1, note);
                    off.setTimeStamp(beat + snapResolution); // Default length = grid size
                    clip->midiSequence.addEvent(off);
                    
                    clip->midiSequence.updateMatchedPairs();
                    repaint();
                }
            }
        }
        repaint();
    }
    
    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (isEditingVelocity)
        {
            editVelocity(e);
            return;
        }

        if (selectedNoteIndex != -1 && isDragging)
        {
            auto* noteOn = clip->midiSequence.getEventPointer(selectedNoteIndex);
            int pairIndex = clip->midiSequence.getIndexOfMatchingKeyUp(selectedNoteIndex);
            auto* noteOff = clip->midiSequence.getEventPointer(pairIndex);
            
            if (!noteOn || !noteOff) return;
            
            int topOffset = 30;
            double currentBeat = xToBeats(e.x);
            int currentPitch = yToPitch(e.y - topOffset);
            
            if (isResizing)
            {
                double diff = currentBeat - dragStartBeat;
                double newDuration = originalNoteDuration + diff;
                
                if (!e.mods.isAltDown())
                    newDuration = std::round(newDuration / snapResolution) * snapResolution;
                    
                if (newDuration < snapResolution) newDuration = snapResolution;
                noteOff->message.setTimeStamp(noteOn->message.getTimeStamp() + newDuration);
            }
            else
            {
                double beatDiff = currentBeat - dragStartBeat;
                int pitchDiff = currentPitch - dragStartPitch;
                
                double newStart = originalNoteStart + beatDiff;
                
                if (!e.mods.isAltDown())
                    newStart = std::round(newStart / snapResolution) * snapResolution;
                    
                if (newStart < 0) newStart = 0;
                
                int newPitch = originalNotePitch + pitchDiff;
                if (newPitch < 0) newPitch = 0;
                if (newPitch > 127) newPitch = 127;
                
                double duration = noteOff->message.getTimeStamp() - noteOn->message.getTimeStamp();
                
                noteOn->message.setTimeStamp(newStart);
                noteOn->message.setNoteNumber(newPitch);
                
                noteOff->message.setTimeStamp(newStart + duration);
                noteOff->message.setNoteNumber(newPitch);
            }
            repaint();
        }
    }
    
    void mouseUp(const juce::MouseEvent& e) override
    {
        if (isDragging || isEditingVelocity)
        {
            if (clip)
            {
                clip->midiSequence.sort();
                clip->midiSequence.updateMatchedPairs();
            }
            isDragging = false;
            isResizing = false;
            isEditingVelocity = false;
            repaint();
        }
    }

    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        if (e.mods.isCtrlDown()) // Zoom
        {
            // Zoom X (Time)
            pixelsPerBeat += wheel.deltaY * 50.0;
            if (pixelsPerBeat < 10.0) pixelsPerBeat = 10.0;
            if (pixelsPerBeat > 500.0) pixelsPerBeat = 500.0;
            
            // Zoom Y (Pitch)
            noteHeight += (int)(wheel.deltaY * 5.0);
            if (noteHeight < 5) noteHeight = 5;
            if (noteHeight > 50) noteHeight = 50;
        }
        else if (e.mods.isShiftDown()) // Scroll X
        {
            scrollX -= wheel.deltaY * 50.0; // Increased speed
            if (scrollX < 0) scrollX = 0;
        }
        else // Scroll Y
        {
            scrollY -= wheel.deltaY * 50.0; // Increased speed
            // Limit scrollY?
        }
        repaint();
    }
    
private:
    ProjectState& projectState;
    Clip* clip = nullptr;
    juce::ComboBox gridSelect;
    
    double pixelsPerBeat = 100.0;
    int noteHeight = 20;
    int keyboardWidth = 60; // Reduced width
    int velocityHeight = 100;
    int scrollX = 0;
    int scrollY = 0;
    
    int selectedNoteIndex = -1;
    bool isDragging = false;
    bool isResizing = false;
    bool isEditingVelocity = false;
    double dragStartBeat = 0;
    int dragStartPitch = 0;
    double originalNoteStart = 0;
    int originalNotePitch = 0;
    double originalNoteDuration = 0;
    
    double snapResolution = 0.25; // 16th note

    int getNoteAt(int x, int y)
    {
        int topOffset = 30;
        for (int i = 0; i < clip->midiSequence.getNumEvents(); ++i)
        {
            auto* event = clip->midiSequence.getEventPointer(i);
            if (event->message.isNoteOn())
            {
                int note = event->message.getNoteNumber();
                double startBeat = event->message.getTimeStamp();
                
                auto* noteOff = clip->midiSequence.getEventPointer(clip->midiSequence.getIndexOfMatchingKeyUp(i));
                double endBeat = noteOff ? noteOff->message.getTimeStamp() : (startBeat + 1.0);
                
                int nx = beatsToX(startBeat);
                int ny = pitchToY(note) + topOffset;
                int nw = beatsToX(endBeat) - nx;
                int nh = noteHeight;
                
                if (x >= nx && x < nx + nw && y >= ny && y < ny + nh)
                {
                    return i;
                }
            }
        }
        return -1;
    }
    
    void editVelocity(const juce::MouseEvent& e)
    {
        double beat = xToBeats(e.x);
        int bestIdx = -1;
        
        for (int i = 0; i < clip->midiSequence.getNumEvents(); ++i)
        {
            auto* event = clip->midiSequence.getEventPointer(i);
            if (event->message.isNoteOn())
            {
                double start = event->message.getTimeStamp();
                int x = beatsToX(start);
                if (std::abs(e.x - x) < 10) 
                {
                    bestIdx = i;
                    break;
                }
            }
        }
        
        if (bestIdx != -1)
        {
            auto* event = clip->midiSequence.getEventPointer(bestIdx);
            float vel = 1.0f - (float)(e.y - (getHeight() - velocityHeight)) / (float)velocityHeight;
            if (vel < 0) vel = 0;
            if (vel > 1) vel = 1;
            event->message.setVelocity(vel);
            selectedNoteIndex = bestIdx;
            repaint();
        }
    }
    
    double xToBeats(int x) const { return (x - keyboardWidth + scrollX) / pixelsPerBeat; }
    int yToPitch(int y) const { return 127 - ((y + scrollY) / noteHeight); }
    int beatsToX(double beats) const { return (int)(beats * pixelsPerBeat) - scrollX + keyboardWidth; }
    int pitchToY(int pitch) const { return (127 - pitch) * noteHeight - scrollY; }
};
