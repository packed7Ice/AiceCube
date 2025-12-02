#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../model/ProjectState.h"
#include "ClipComponent.h"

class TimelineComponent : public juce::Component, public juce::KeyListener
{
public:
    TimelineComponent(ProjectState& state);
    ~TimelineComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;

    void updateTimeline();
    
    std::function<void(Clip&)> onClipEditRequested;

private:
    ProjectState& projectState;
    juce::OwnedArray<ClipComponent> clipComponents;
    
    double pixelsPerBeat = 40.0;
    int trackHeight = 60;
    double scrollX = 0.0;
    
    // Snap
    bool snapEnabled = true;
    double snapResolution = 1.0; // 1 beat
    
    // Selection
    juce::Rectangle<int> selectionRect;
    bool isSelecting = false;
    std::vector<Clip*> selectedClips;
    
    // Automation Editing
    int draggingAutomationTrackIndex = -1;
    int draggingAutomationPointIndex = -1;
    const float automationPointRadius = 6.0f;
    
    // Loop Editing
    bool isDraggingLoop = false;
    double loopDragStartBeat = 0.0;
    
    // Helpers
    double xToBeats(int x) const { return (x + scrollX) / pixelsPerBeat; }
    int beatsToX(double beats) const { return (int)(beats * pixelsPerBeat - scrollX); }
    int yToTrackIndex(int y) const { return y / trackHeight; }
    
    void selectClipsInRect(const juce::Rectangle<int>& rect);
    void deleteSelectedClips();
    void duplicateSelectedClips();
    void splitClipAtPlayhead();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineComponent)
};
