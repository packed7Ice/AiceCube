#include "MainComponent.h"
#include "model/ProjectSerializer.h"
#include "components/SettingsComponent.h"

MainComponent::MainComponent()
    : audioEngine(projectState),
      commandExecutor(projectState, apiClient),
      transportBar(projectState),
      trackHeaders(projectState, audioEngine),
      timeline(projectState),
      mixer(projectState, audioEngine),
      pianoRoll(projectState)
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
    
    transportBar.onSaveClicked = [this] { saveProject(); };
    
    transportBar.onSettingsClicked = [this] {
        auto* settings = new SettingsComponent(deviceManager, audioEngine);
        juce::DialogWindow::LaunchOptions opt;
        opt.content.setOwned(settings);
        opt.dialogTitle = "Audio Settings";
        opt.dialogBackgroundColour = juce::Colours::darkgrey;
        opt.resizable = true;
        opt.content->setSize(500, 600);
        opt.launchAsync();
    };
    


    // Timeline Callbacks
    // Timeline Callbacks
    timeline.onClipEditRequested = [this](Clip& clip) {
        if (clip.isMidi)
        {
            pianoRoll.setClip(&clip);
            showPianoRoll = true;
            showMixer = false;
            transportBar.setMixerButtonState(showMixer);
            transportBar.setEditorButtonState(showPianoRoll);
            resized();
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

    // Transport Callbacks
    transportBar.onMixerClicked = [this] {
        showMixer = !showMixer;
        if (showMixer) showPianoRoll = false;
        transportBar.setMixerButtonState(showMixer);
        transportBar.setEditorButtonState(showPianoRoll);
        resized();
    };

    transportBar.onEditorClicked = [this] {
        showPianoRoll = !showPianoRoll;
        if (showPianoRoll) showMixer = false;
        transportBar.setMixerButtonState(showMixer);
        transportBar.setEditorButtonState(showPianoRoll);
        resized();
    };

    // Initialize Plugins
    // audioEngine.scanPlugins();
    audioEngine.scanPluginsAsync(nullptr, nullptr);
    
    setSize(1600, 900);

    // Audio
    setAudioChannels(0, 2);

    // UI
    addAndMakeVisible(transportBar);
    addAndMakeVisible(trackHeaders);
    trackHeaders.onTrackListChanged = [this] {
        timeline.updateTimeline();
        timeline.repaint();
    };
    addAndMakeVisible(timeline);
    addAndMakeVisible(mixer);
    addAndMakeVisible(pianoRoll);
    pianoRoll.setVisible(false); // Hidden by default
    pianoRoll.setVisible(false); // Hidden by default
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

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::spaceKey)
    {
        if (projectState.isPlaying)
            transportBar.onStopClicked();
        else
            transportBar.onPlayClicked();
        return true;
    }
    return false;
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    
    // Top: Transport
    transportBar.setBounds(area.removeFromTop(40));
    
    // Right: Agent Panel
    agentPanel.setBounds(area.removeFromRight(300));
    
    // Bottom: Mixer or Piano Roll (Overlay or Split)
    // If shown, take some space from bottom
    if (showPianoRoll)
    {
        auto bottomArea = area.removeFromBottom(300);
        pianoRoll.setVisible(true);
        pianoRoll.setBounds(bottomArea);
        mixer.setVisible(false);
    }
    else if (showMixer)
    {
        auto bottomArea = area.removeFromBottom(300);
        mixer.setVisible(true);
        mixer.setBounds(bottomArea);
        pianoRoll.setVisible(false);
    }
    else
    {
        pianoRoll.setVisible(false);
        mixer.setVisible(false);
    }

    // Center: Tracks + Timeline
    auto trackHeaderWidth = 200;
    trackHeaders.setBounds(area.removeFromLeft(trackHeaderWidth));
    timeline.setBounds(area);
}

void MainComponent::saveProject()
{
    fileChooser = std::make_unique<juce::FileChooser>("Save Project",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.aice");
        
    auto folderChooserFlags = juce::FileBrowserComponent::saveMode | 
                              juce::FileBrowserComponent::canSelectFiles |
                              juce::FileBrowserComponent::warnAboutOverwriting;
                              
    fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file != juce::File{})
        {
            auto json = ProjectSerializer::toJSON(projectState);
            file.replaceWithText(json);
        }
    });
}

void MainComponent::loadProject()
{
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
            
            // Restore plugins
            audioEngine.updateGraph();
            for (auto& track : projectState.tracks)
            {
                if (track->instrumentPlugin && track->instrumentPlugin->identifier.isNotEmpty())
                {
                    juce::PluginDescription desc;
                    desc.fileOrIdentifier = track->instrumentPlugin->identifier;
                    audioEngine.setInstrumentPlugin(track.get(), desc);
                }
            }
            
            resized(); // Re-layout
            timeline.updateTimeline();
            trackHeaders.updateTrackList();
        }
    });
}

void MainComponent::importMidi()
{
    fileChooser = std::make_unique<juce::FileChooser>("Import MIDI",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.mid;*.midi");
        
    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                juce::MidiFile midiFile;
                juce::FileInputStream stream(file);
                if (midiFile.readFrom(stream))
                {
                    // midiFile.convertTimestampType(true); // Removed to fix build
                    double ticksPerQuarter = midiFile.getTimeFormat();
                    if (ticksPerQuarter < 0) ticksPerQuarter = 960.0; // SMPTE fallback
                    
                    for (int i = 0; i < midiFile.getNumTracks(); ++i)
                    {
                        auto* trackSeq = midiFile.getTrack(i);
                        auto track = projectState.addTrack(TrackType::Midi, "Imported MIDI " + juce::String(i+1));
                        
                        Clip clip;
                        clip.name = "MIDI Clip";
                        clip.startBeat = 0;
                        clip.lengthBeats = midiFile.getLastTimestamp() / ticksPerQuarter;
                        if (clip.lengthBeats <= 0) clip.lengthBeats = 4.0;
                        
                        clip.isMidi = true;
                        
                        // Convert events to beats
                        for (int j = 0; j < trackSeq->getNumEvents(); ++j)
                        {
                            auto msg = trackSeq->getEventPointer(j)->message;
                            msg.setTimeStamp(msg.getTimeStamp() / ticksPerQuarter);
                            clip.midiSequence.addEvent(msg);
                        }
                        clip.midiSequence.updateMatchedPairs();
                        
                        clip.trackIndex = 0;
                        track->clips.push_back(clip);
                    }
                    
                    juce::MessageManager::callAsync([this] {
                        trackHeaders.updateTrackList();
                        timeline.updateTimeline();
                        timeline.repaint();
                    });
                }
            }
        });
}

void MainComponent::exportMidi()
{
    fileChooser = std::make_unique<juce::FileChooser>("Export MIDI",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.mid");
        
    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
        [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File{})
            {
                juce::MidiFile midiFile;
                midiFile.setTicksPerQuarterNote(960);
                
                for (const auto& track : projectState.tracks)
                {
                    if (track->type == TrackType::Midi)
                    {
                        juce::MidiMessageSequence trackSeq;
                        for (const auto& clip : track->clips)
                        {
                            if (clip.isMidi)
                            {
                                auto& seq = clip.midiSequence;
                                for (int i = 0; i < seq.getNumEvents(); ++i)
                                {
                                    auto msg = seq.getEventPointer(i)->message;
                                    // Convert beats to ticks (960 TPQ)
                                    msg.setTimeStamp((msg.getTimeStamp() + clip.startBeat) * 960.0);
                                    trackSeq.addEvent(msg);
                                }
                            }
                        }
                        trackSeq.updateMatchedPairs();
                        midiFile.addTrack(trackSeq);
                    }
                }
                
                juce::FileOutputStream stream(file);
                midiFile.writeTo(stream);
            }
        });
}


