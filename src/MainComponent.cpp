#include "MainComponent.h"
#include "model/ProjectSerializer.h"

MainComponent::MainComponent()
    : audioEngine(projectState),
      commandExecutor(projectState, apiClient),
      transportBar(projectState),
      trackHeaders(projectState),
      timeline(projectState),
      mixer(projectState, audioEngine)
{
    juce::LookAndFeel::setDefaultLookAndFeel(&modernLookAndFeel);
    
    // Transport Callbacks
    transportBar.onPlayClicked = [this] {
        projectState.isPlaying = true;
    };
    
    transportBar.onStopClicked = [this] {
        projectState.isPlaying = false;
        projectState.playheadBeat = 0.0;
        audioEngine.stopRecording(); // Stop recording
        timeline.updateTimeline(); // Refresh clips
        timeline.repaint();
    };
    
    transportBar.onRecordClicked = [this] {
        audioEngine.startRecording();
        projectState.isPlaying = true;
    };
    
    transportBar.onSaveClicked = [this] {
        fileChooser = std::make_unique<juce::FileChooser>("Save Project",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.aice");
            
        auto folderChooserFlags = juce::FileBrowserComponent::saveMode | 
                                  juce::FileBrowserComponent::canSelectFiles;
                                  
        fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File{})
            {
                auto json = ProjectSerializer::toJSON(projectState);
                file.replaceWithText(json);
            }
        });
    };
    
    transportBar.onLoadClicked = [this] {
        fileChooser = std::make_unique<juce::FileChooser>("Load Project",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.aice");
            
        auto folderChooserFlags = juce::FileBrowserComponent::openMode | 
                                  juce::FileBrowserComponent::canSelectFiles;
                                  
        fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File{})
            {
                auto json = file.loadFileAsString();
                ProjectSerializer::fromJSON(projectState, json);
                
                // Refresh UI
                trackHeaders.updateTrackList();
                timeline.updateTimeline();
                mixer.updateMixer();
                
                // Restore Plugins (Basic)
                for (auto& track : projectState.tracks)
                {
                    // Restore Instrument
                    if (track->instrumentPlugin && track->instrumentPlugin->identifier.isNotEmpty())
                    {
                        juce::PluginDescription desc;
                        desc.fileOrIdentifier = track->instrumentPlugin->identifier;
                        audioEngine.setInstrumentPlugin(track.get(), desc);
                        
                        // Apply State
                        if (track->instrumentPlugin->instance && track->instrumentPlugin->state.getSize() > 0)
                        {
                            track->instrumentPlugin->instance->setStateInformation(track->instrumentPlugin->state.getData(), (int)track->instrumentPlugin->state.getSize());
                        }
                    }
                    
                    // Restore Inserts
                    for (int i = 0; i < track->insertPlugins.size(); ++i)
                    {
                        auto& slot = track->insertPlugins[i];
                        if (slot && slot->identifier.isNotEmpty())
                        {
                            juce::PluginDescription desc;
                            desc.fileOrIdentifier = slot->identifier;
                            audioEngine.addPluginToTrack(track.get(), i, desc);
                            
                            // Apply State
                            if (slot->instance && slot->state.getSize() > 0)
                            {
                                slot->instance->setStateInformation(slot->state.getData(), (int)slot->state.getSize());
                            }
                        }
                    }
                }
                
                resized(); // Re-layout
            }
        });
    };

    // Timeline Callbacks
    timeline.onClipEditRequested = [this](Clip& clip) {
        if (clip.isMidi)
        {
            auto* editor = new PianoRollEditorComponent(clip);
            juce::DialogWindow::LaunchOptions opt;
            opt.content.setOwned(editor);
            opt.dialogTitle = "Piano Roll - " + clip.name;
            opt.dialogBackgroundColour = juce::Colours::darkgrey;
            opt.resizable = true;
            opt.launchAsync();
        }
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

    // Initialize Plugins
    audioEngine.scanPlugins();
    
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
}

MainComponent::~MainComponent()
{
    shutdownAudio();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
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
