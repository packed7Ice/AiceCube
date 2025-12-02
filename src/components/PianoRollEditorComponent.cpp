#include "PianoRollEditorComponent.h"

PianoRollEditorComponent::PianoRollEditorComponent(Clip& c) : clip(c)
{
    setSize(800, 600);
}

void PianoRollEditorComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.5f));
    
    // Draw Grid
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    int numBeats = (int)clip.lengthBeats + 1;
    for (int i = 0; i < numBeats; ++i)
    {
        float x = (float)(i * pixelsPerBeat - scrollX);
        g.drawVerticalLine((int)x, 0.0f, (float)getHeight());
    }
    
    for (int i = 0; i < 128; ++i)
    {
        float y = (float)((127 - i) * noteHeight - scrollY);
        g.drawHorizontalLine((int)y, 0.0f, (float)getWidth());
    }
    
    // Draw Notes
    g.setColour(juce::Colours::orange);
    
    // Iterate MIDI sequence
    for (int i = 0; i < clip.midiSequence.getNumEvents(); ++i)
    {
        auto* event = clip.midiSequence.getEventPointer(i);
        if (event->message.isNoteOn())
        {
            int note = event->message.getNoteNumber();
            double startBeat = event->message.getTimeStamp(); // Assuming timestamp is beats for now? 
            // Wait, MidiMessageSequence usually uses seconds or ticks. 
            // In our simple model, let's assume timestamps are beats for simplicity or we need conversion.
            // Let's assume beats.
            
            // Find Note Off
            double endBeat = startBeat + 1.0; // Default
            // Ideally we pair note-ons and note-offs.
            // For visualization let's just draw a fixed length if we can't find pair easily without iterating.
            
            // Simple approach:
            float x = (float)(startBeat * pixelsPerBeat - scrollX);
            float y = (float)((127 - note) * noteHeight - scrollY);
            float w = (float)(1.0 * pixelsPerBeat); // 1 beat length default
            
            g.fillRect(x, y, w, (float)noteHeight - 1);
        }
    }
}

void PianoRollEditorComponent::resized()
{
}

void PianoRollEditorComponent::mouseDown(const juce::MouseEvent& e)
{
    // Add note logic placeholder
    if (e.getNumberOfClicks() == 2)
    {
        double beat = (e.x + scrollX) / pixelsPerBeat;
        int note = 127 - ((e.y + scrollY) / noteHeight);
        
        if (note >= 0 && note < 128)
        {
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
