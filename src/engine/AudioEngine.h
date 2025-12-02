#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../model/ProjectState.h"

class AudioEngine
{
public:
    AudioEngine(ProjectState& state);
    ~AudioEngine();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();
    
    // Render audio and process graph
    void processAudio(const juce::AudioSourceChannelInfo& bufferToFill, juce::MidiBuffer& midiMessages);

    juce::AudioProcessorGraph& getGraph() { return *mainProcessor; }
    
    // Graph management
    void updateGraph(); // Syncs graph with ProjectState
    
    // File Management
    juce::AudioFormatManager& getFormatManager() { return formatManager; }

private:
    ProjectState& projectState;
    std::unique_ptr<juce::AudioProcessorGraph> mainProcessor;
    juce::AudioFormatManager formatManager;
    
    std::map<juce::String, std::unique_ptr<juce::AudioFormatReader>> fileReaders;
    
    juce::AudioFormatReader* getReaderFor(const juce::File& file);

    // Nodes
    juce::AudioProcessorGraph::Node::Ptr audioInputNode;
    juce::AudioProcessorGraph::Node::Ptr audioOutputNode;
    juce::AudioProcessorGraph::Node::Ptr midiInputNode;
    juce::AudioProcessorGraph::Node::Ptr midiOutputNode;
    
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
};
