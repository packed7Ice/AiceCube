#include "MainComponent.h"

MainComponent::MainComponent()
    : audioEngine(projectState),
      commandExecutor(projectState, apiClient),
      transportBar(projectState),
      trackHeaders(projectState),
      timeline(projectState),
      mixer(projectState)
{
    setSize(1200, 800);

    // Audio
    setAudioChannels(0, 2);

    // UI
    addAndMakeVisible(transportBar);
    addAndMakeVisible(trackHeaders);
    addAndMakeVisible(timeline);
    addAndMakeVisible(mixer);
    addAndMakeVisible(browser);
    addAndMakeVisible(agentPanel);
    
    // Transport Callbacks
    transportBar.onPlayClicked = [this] {
        projectState.isPlaying = true;
    };
    transportBar.onStopClicked = [this] {
        projectState.isPlaying = false;
        projectState.playheadBeat = 0.0;
        timeline.repaint();
    };
    
    // Agent Logic
    agentPanel.onCommandEntered = [this](const juce::String& command) {
        auto commands = AgentLogic::interpretInstruction(command);
        for (const auto& cmd : commands)
        {
            commandExecutor.execute(cmd, 
                [this](const juce::String& msg) { agentPanel.logMessage(msg); },
                [this] { 
                    juce::MessageManager::callAsync([this] {
                        trackHeaders.updateTrackList();
                        timeline.updateTimeline();
                    });
                }
            );
        }
    };
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    audioEngine.prepareToPlay(sampleRate, samplesPerBlockExpected);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    juce::MidiBuffer midiMessages;
    audioEngine.processAudio(bufferToFill, midiMessages);
    
    // Update playhead if playing
    if (projectState.isPlaying)
    {
        double samplesPerBeat = (60.0 / projectState.tempo) * audioEngine.getGraph().getSampleRate();
        double beatsInBlock = bufferToFill.numSamples / samplesPerBeat;
        projectState.playheadBeat += beatsInBlock;
        
        juce::MessageManager::callAsync([this] { timeline.repaint(); });
    }
}

void MainComponent::releaseResources()
{
    audioEngine.releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    
    // Top: Transport
    transportBar.setBounds(area.removeFromTop(40));
    
    // Bottom: Mixer
    mixer.setBounds(area.removeFromBottom(200));
    
    // Left: Browser
    browser.setBounds(area.removeFromLeft(250));
    
    // Right: Agent Panel
    agentPanel.setBounds(area.removeFromRight(300));
    
    // Center: Tracks + Timeline
    auto trackHeaderWidth = 200;
    trackHeaders.setBounds(area.removeFromLeft(trackHeaderWidth));
    timeline.setBounds(area);
}
