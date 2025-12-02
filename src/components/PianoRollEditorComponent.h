#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../model/MusicData.h"

class PianoRollEditorComponent : public juce::Component
{
public:
    PianoRollEditorComponent(Clip& c) : clip(c)
    {
        setSize(800, 600);
    }
    
    ~PianoRollEditorComponent() override {}

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey.darker(0.5f));
        
        auto bounds = getLocalBounds();
        auto keyboardArea = bounds.removeFromLeft(keyboardWidth);
        auto contentArea = bounds;
        
        // 1. Draw Grid (Content Area)
        g.reduceClipRegion(contentArea);
        
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        int numBeats = (int)clip.lengthBeats + 1;
        for (int i = 0; i < numBeats; ++i)
        {
            float x = (float)beatsToX(i);
            g.drawVerticalLine((int)x, 0.0f, (float)getHeight());
        }
        
        for (int i = 0; i < 128; ++i)
        {
            float y = (float)pitchToY(i);
            g.setColour((i % 12 == 0) ? juce::Colours::white.withAlpha(0.2f) : juce::Colours::white.withAlpha(0.05f));
            g.drawHorizontalLine((int)y, (float)keyboardWidth, (float)getWidth());
        }
        
        // 2. Draw Notes
        for (int i = 0; i < clip.midiSequence.getNumEvents(); ++i)
        {
            auto* event = clip.midiSequence.getEventPointer(i);
            if (event->message.isNoteOn())
            {
                int note = event->message.getNoteNumber();
                double startBeat = event->message.getTimeStamp();
                
                auto* noteOff = clip.midiSequence.getEventPointer(clip.midiSequence.getIndexOfMatchingKeyUp(i));
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
    }
    
    void resized() override {}
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.x < keyboardWidth) return;
        
        int noteIdx = getNoteAt(e.x, e.y);
        
        if (noteIdx != -1)
        {
            selectedNoteIndex = noteIdx;
            isDragging = true;
            
            auto* event = clip.midiSequence.getEventPointer(noteIdx);
            originalNoteStart = event->message.getTimeStamp();
            originalNotePitch = event->message.getNoteNumber();
            
            dragStartBeat = xToBeats(e.x);
            dragStartPitch = yToPitch(e.y);
            
            double noteEnd = originalNoteStart + 1.0;
            auto* noteOff = clip.midiSequence.getEventPointer(clip.midiSequence.getIndexOfMatchingKeyUp(noteIdx));
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
                    clip.midiSequence.addEvent(m);
                    
                    auto off = juce::MidiMessage::noteOff(1, note);
                    off.setTimeStamp(beat + 1.0);
                    clip.midiSequence.addEvent(off);
                    
                    clip.midiSequence.updateMatchedPairs();
                    repaint();
                }
            }
        }
        repaint();
    }
    
    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (selectedNoteIndex != -1 && isDragging)
        {
            auto* noteOn = clip.midiSequence.getEventPointer(selectedNoteIndex);
            int pairIndex = clip.midiSequence.getIndexOfMatchingKeyUp(selectedNoteIndex);
            auto* noteOff = clip.midiSequence.getEventPointer(pairIndex);
            
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
                newStart = std::round(newStart * 4.0) / 4.0;
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
        if (isDragging)
        {
            clip.midiSequence.sort();
            clip.midiSequence.updateMatchedPairs();
            isDragging = false;
            isResizing = false;
            selectedNoteIndex = -1;
            repaint();
        }
    }

private:
    Clip& clip;
    double pixelsPerBeat = 100.0;
    int noteHeight = 20;
    int keyboardWidth = 60;
    int scrollX = 0;
    int scrollY = 0;
    
    int selectedNoteIndex = -1;
    bool isDragging = false;
    bool isResizing = false;
    double dragStartBeat = 0;
    int dragStartPitch = 0;
    double originalNoteStart = 0;
    int originalNotePitch = 0;
    double originalNoteDuration = 0;

    int getNoteAt(int x, int y)
    {
        for (int i = 0; i < clip.midiSequence.getNumEvents(); ++i)
        {
            auto* event = clip.midiSequence.getEventPointer(i);
            if (event->message.isNoteOn())
            {
                int note = event->message.getNoteNumber();
                double startBeat = event->message.getTimeStamp();
                
                auto* noteOff = clip.midiSequence.getEventPointer(clip.midiSequence.getIndexOfMatchingKeyUp(i));
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
    
    double xToBeats(int x) const { return (x - keyboardWidth + scrollX) / pixelsPerBeat; }
    int yToPitch(int y) const { return 127 - ((y + scrollY) / noteHeight); }
    int beatsToX(double beats) const { return (int)(beats * pixelsPerBeat) - scrollX + keyboardWidth; }
    int pitchToY(int pitch) const { return (127 - pitch) * noteHeight - scrollY; }
};
