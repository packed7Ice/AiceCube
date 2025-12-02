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
    // Plugin Management
    juce::AudioPluginFormatManager& getPluginFormatManager() { return pluginFormatManager; }
    juce::KnownPluginList& getKnownPluginList() { return knownPluginList; }
    
    void scanPlugins();
    std::shared_ptr<juce::AudioPluginInstance> loadPlugin(const juce::PluginDescription& desc);
    
    void addPluginToTrack(Track* track, int slotIndex, const juce::PluginDescription& desc);
    void setInstrumentPlugin(Track* track, const juce::PluginDescription& desc);
    void showPluginWindow(juce::AudioPluginInstance* plugin);
    void closePluginWindow(juce::AudioPluginInstance* plugin);

    // Recording
    void startRecording();
    void stopRecording();
    bool isRecording = false;

private:
    ProjectState& projectState;
    std::unique_ptr<juce::AudioProcessorGraph> mainProcessor;
    juce::AudioFormatManager formatManager;
    
    // Recording
    juce::TimeSliceThread backgroundThread { "Audio Recorder Thread" };
    std::map<Track*, std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter>> trackRecorders;
    double recordingStartBeat = 0.0;
    std::map<Track*, juce::File> recordingFiles; // Keep track of files to create clips
    
    // Plugins
    juce::AudioPluginFormatManager pluginFormatManager;
    juce::KnownPluginList knownPluginList;
    std::unique_ptr<juce::PluginDirectoryScanner> pluginScanner;
    juce::File knownPluginListFile;
    
    // Plugin Windows
    // We use a map to keep track of windows for each plugin instance
    // Key is the raw pointer to the plugin instance (which is owned by Track/PluginSlot)
    std::map<juce::AudioPluginInstance*, juce::Component::SafePointer<juce::DocumentWindow>> pluginWindows;
    
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
