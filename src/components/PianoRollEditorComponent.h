#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../model/MusicData.h"

class PianoRollEditorComponent : public juce::Component
{
public:
    PianoRollEditorComponent()
    {
        setSize(800, 600);
    }
    
    void setClip(Clip* c)
    {
        clip = c;
        repaint();
    }
    
    ~PianoRollEditorComponent() override {}

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
        auto velocityArea = bounds.removeFromBottom(velocityHeight);
        auto keyboardArea = bounds.removeFromLeft(keyboardWidth);
        auto contentArea = bounds;
        
        // 1. Draw Grid (Content Area)
        g.reduceClipRegion(contentArea);
        
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        int numBeats = (int)clip->lengthBeats + 1;
        for (int i = 0; i < numBeats; ++i)
        {
            float x = (float)beatsToX(i);
            g.drawVerticalLine((int)x, 0.0f, (float)contentArea.getBottom());
        }
        
        for (int i = 0; i < 128; ++i)
        {
            float y = (float)pitchToY(i);
            g.setColour((i % 12 == 0) ? juce::Colours::white.withAlpha(0.2f) : juce::Colours::white.withAlpha(0.05f));
            g.drawHorizontalLine((int)y, (float)keyboardWidth, (float)getWidth());
        }
        
        // 2. Draw Notes
        for (int i = 0; i < clip->midiSequence.getNumEvents(); ++i)
        {
            auto* event = clip->midiSequence.getEventPointer(i);
            if (event->message.isNoteOn())
            {
                int note = event->message.getNoteNumber();
                double startBeat = event->message.getTimeStamp();
                
                auto* noteOff = clip->midiSequence.getEventPointer(clip->midiSequence.getIndexOfMatchingKeyUp(i));
                double endBeat = noteOff ? noteOff->message.getTimeStamp() : (startBeat + 1.0);
                
                double duration = endBeat - startBeat;
                
                float x = (float)beatsToX(startBeat);
                float y = (float)pitchToY(note);
                float w = (float)(duration * pixelsPerBeat);
                
                if (i == selectedNoteIndex)
                    g.setColour(juce::Colours::orange.brighter());
                else
                    g.setColour(juce::Colours::orange);
                
                g.fillRect(x, y, w, (float)noteHeight - 1);
                
                g.setColour(juce::Colours::black);
                g.drawRect(x, y, w, (float)noteHeight - 1);
            }
        }
        
        // 3. Draw Velocity Lane
        g.reduceClipRegion(getLocalBounds()); // Reset clip
        g.setColour(juce::Colours::black);
        g.fillRect(velocityArea);
        g.setColour(juce::Colours::white);
        g.drawHorizontalLine(velocityArea.getY(), 0.0f, (float)getWidth());
        
        for (int i = 0; i < clip->midiSequence.getNumEvents(); ++i)
        {
            auto* event = clip->midiSequence.getEventPointer(i);
            if (event->message.isNoteOn())
            {
                double startBeat = event->message.getTimeStamp();
                float x = (float)beatsToX(startBeat);
                float vel = event->message.getVelocity() / 127.0f;
                float h = vel * velocityHeight;
                float y = (float)velocityArea.getBottom() - h;
                
                if (i == selectedNoteIndex)
                    g.setColour(juce::Colours::orange.brighter());
                else
                    g.setColour(juce::Colours::orange);
                    
                g.fillRect(x, y, 5.0f, h);
            }
        }
    }
    
    void resized() override {}
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (!clip) return;

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
            dragStartPitch = yToPitch(e.y);
            
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
                int note = yToPitch(e.y);
                
                if (note >= 0 && note < 128)
                {
                    beat = std::round(beat * 4.0) / 4.0;
                    
                    auto m = juce::MidiMessage::noteOn(1, note, (juce::uint8)100);
                    m.setTimeStamp(beat);
                    clip->midiSequence.addEvent(m);
                    
                    auto off = juce::MidiMessage::noteOff(1, note);
                    off.setTimeStamp(beat + 1.0);
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
            
            double currentBeat = xToBeats(e.x);
            int currentPitch = yToPitch(e.y);
            
            if (isResizing)
            {
                double diff = currentBeat - dragStartBeat;
                double newDuration = originalNoteDuration + diff;
                if (newDuration < 0.1) newDuration = 0.1;
                noteOff->message.setTimeStamp(noteOn->message.getTimeStamp() + newDuration);
            }
            else
            {
                double beatDiff = currentBeat - dragStartBeat;
                int pitchDiff = currentPitch - dragStartPitch;
                
                double newStart = originalNoteStart + beatDiff;
                if (snapEnabled) newStart = std::round(newStart / snapResolution) * snapResolution;
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
            // selectedNoteIndex = -1; // Keep selection
            repaint();
        }
    }

    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        if (e.mods.isCommandDown()) // Zoom X
        {
            pixelsPerBeat += wheel.deltaY * 10.0;
            if (pixelsPerBeat < 10.0) pixelsPerBeat = 10.0;
            if (pixelsPerBeat > 300.0) pixelsPerBeat = 300.0;
        }
        else if (e.mods.isShiftDown()) // Zoom Y
        {
            noteHeight += (int)(wheel.deltaY * 5.0);
            if (noteHeight < 5) noteHeight = 5;
            if (noteHeight > 50) noteHeight = 50;
        }
        else // Scroll
        {
            scrollX -= wheel.deltaX * 20.0;
            scrollY -= wheel.deltaY * 20.0;
            if (scrollX < 0) scrollX = 0;
            // Limit scrollY?
        }
        repaint();
    }
    
    void quantize()
    {
        if (!clip) return;
        for (int i = 0; i < clip->midiSequence.getNumEvents(); ++i)
        {
            auto* event = clip->midiSequence.getEventPointer(i);
            if (event->message.isNoteOn())
            {
                double start = event->message.getTimeStamp();
                double quantized = std::round(start / snapResolution) * snapResolution;
                
                auto* noteOff = clip->midiSequence.getEventPointer(clip->midiSequence.getIndexOfMatchingKeyUp(i));
                double duration = noteOff ? (noteOff->message.getTimeStamp() - start) : 1.0;
                
                event->message.setTimeStamp(quantized);
                if (noteOff) noteOff->message.setTimeStamp(quantized + duration);
            }
        }
        clip->midiSequence.sort();
        clip->midiSequence.updateMatchedPairs();
        repaint();
    }

private:
    Clip* clip = nullptr;
    double pixelsPerBeat = 100.0;
    int noteHeight = 20;
    int keyboardWidth = 200;
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
    
    bool snapEnabled = true;
    double snapResolution = 0.25; // 16th note

    int getNoteAt(int x, int y)
    {
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
                int ny = pitchToY(note);
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
        // Find note at X
        double beat = xToBeats(e.x);
        // Simple: Find closest note start? Or note covering this beat?
        // For now, let's just find the note that starts closest to this beat within a threshold?
        // Or better: if we have a selection, edit that. If not, find note under cursor X.
        
        // Let's iterate and find note at this X
        int bestIdx = -1;
        double minDist = 1000.0;
        
        for (int i = 0; i < clip->midiSequence.getNumEvents(); ++i)
        {
            auto* event = clip->midiSequence.getEventPointer(i);
            if (event->message.isNoteOn())
            {
                double start = event->message.getTimeStamp();
                int x = beatsToX(start);
                if (std::abs(e.x - x) < 10) // Within 10 pixels
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
