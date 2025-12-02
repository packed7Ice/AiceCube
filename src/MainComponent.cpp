#include "MainComponent.h"
#include "AgentLogic.h"

MainComponent::MainComponent() {
  setSize(1280, 768); // Wider default
  
  addAndMakeVisible(transportBar);
  addAndMakeVisible(leftPane);
  addAndMakeVisible(timeline);
  addAndMakeVisible(rightPane);
  addChildComponent(pianoRoll); // Initially hidden

  // Timeline Logic
  timeline.onClipDoubleClicked = [this](Clip* clip) {
      pianoRoll.setClip(clip);
      showPianoRoll = true;
      timeline.setVisible(false);
      pianoRoll.setVisible(true);
      resized();
  };
  
  // Piano Roll Logic
  pianoRoll.onBackClicked = [this] {
      showPianoRoll = false;
      pianoRoll.setVisible(false);
      timeline.setVisible(true);
      resized();
  };

  // Transport Logic
  transportBar.onPlayClicked = [this] {
      appState.isPlaying = true;
  };
  transportBar.onStopClicked = [this] {
      appState.isPlaying = false;
      appState.playheadBeat = 0.0;
      synth.allNotesOff();
      timeline.repaint();
  };
  
  transportBar.onImportAudioClicked = [this] {
      importAudio();
  };

  // Agent Logic (Connect RightPane's AgentPanel)
  rightPane.getAgentPanel().onCommandEntered = [this](const juce::String& command) {
      auto results = AgentLogic::interpretInstruction(command);
      if (results.isEmpty())
      {
          rightPane.getAgentPanel().logMessage("No command recognized.");
      }
      else
      {
          for (const auto& cmd : results)
          {
              rightPane.getAgentPanel().logMessage("Parsed: " + cmd.toString());
              
              commandExecutor.execute(cmd, 
                [this](const juce::String& msg) { rightPane.getAgentPanel().logMessage(msg); },
                [this]() { 
                    juce::MessageManager::callAsync([this] { timeline.repaint(); });
                }
              );
          }
      }
  };

  // Initialize Audio
  setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    this->sampleRate = sampleRate;
    audioEngine.prepareToPlay(sampleRate, samplesPerBlockExpected);
    synth.prepareToPlay(sampleRate, samplesPerBlockExpected);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // 1. Render Audio Engine (Audio Clips + Graph)
    // This clears the buffer and renders audio clips
    juce::MidiBuffer midiBuffer;
    audioEngine.processAudio(bufferToFill, midiBuffer);

    if (appState.isPlaying)
    {
        double samplesPerBeat = (60.0 / appState.tempoBpm) * sampleRate;
        int numSamples = bufferToFill.numSamples;
        
        for (int i = 0; i < numSamples; ++i)
        {
            double currentBeat = appState.playheadBeat;
            double nextBeat = currentBeat + (1.0 / samplesPerBeat);

            // Check for solo state
            bool anySolo = false;
            for (const auto& track : appState.tracks)
            {
                if (track.isSoloed)
                {
                    anySolo = true;
                    break;
                }
            }

            // Check note starts/ends across all tracks/clips
            for (const auto& track : appState.tracks)
            {
                // Skip if muted or (solo exists and this track is not soloed)
                if (track.isMuted || (anySolo && !track.isSoloed))
                {
                    continue;
                }

                for (const auto& clip : track.clips)
                {
                    if (clip.type != ClipType::Midi) continue;

                    // Relative beat within clip
                    double clipRelBeat = currentBeat - clip.startBeat;
                    double clipRelNextBeat = nextBeat - clip.startBeat;

                    if (clipRelBeat >= 0 && clipRelBeat < clip.lengthBeats)
                    {
                        for (const auto& note : clip.notes)
                        {
                            // Note start
                            if (note.start >= clipRelBeat && note.start < clipRelNextBeat)
                            {
                                synth.noteOn(note.pitch, note.velocity / 127.0f * track.volume);
                            }
                            // Note end
                            if ((note.start + note.duration) >= clipRelBeat && (note.start + note.duration) < clipRelNextBeat)
                            {
                                synth.noteOff(note.pitch);
                            }
                        }
                    }
                }
            }
            
            appState.playheadBeat = nextBeat;
        }
        
        // Render Synth (Adds to buffer)
        juce::AudioBuffer<float> synthBuffer(bufferToFill.buffer->getArrayOfWritePointers(), 
                                             bufferToFill.buffer->getNumChannels(), 
                                             bufferToFill.startSample, 
                                             bufferToFill.numSamples);
        synth.renderNextBlock(synthBuffer, 0, numSamples);
        
        // Update UI (throttled in real app, but ok here)
        juce::MessageManager::callAsync([this] { timeline.repaint(); });
    }
}

void MainComponent::releaseResources()
{
    audioEngine.releaseResources();
    synth.allNotesOff();
}

void MainComponent::importAudio()
{
    fChooser = std::make_unique<juce::FileChooser>("Select Audio File", juce::File(), "*.wav;*.aiff;*.flac;*.mp3");
    fChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& chooser) {
            auto file = chooser.getResult();
            if (file.exists())
            {
                Clip clip;
                clip.type = ClipType::Audio;
                clip.name = file.getFileName();
                clip.audioFile = file;
                
                // Calculate length
                auto* reader = audioEngine.getFormatManager().createReaderFor(file);
                if (reader)
                {
                    double duration = reader->lengthInSamples / reader->sampleRate;
                    double bpm = appState.tempoBpm;
                    clip.lengthBeats = duration * (bpm / 60.0);
                    delete reader;
                }
                else
                {
                    clip.lengthBeats = 4.0; // Fallback
                }
                
                // Add to first audio track or create one
                int targetTrack = -1;
                for(int i=0; i<appState.tracks.size(); ++i) {
                    if(appState.tracks[i].type == TrackType::Audio) {
                        targetTrack = i;
                        break;
                    }
                }
                
                if (targetTrack == -1) {
                    appState.addTrack("Audio Track", TrackType::Audio);
                    targetTrack = appState.tracks.size() - 1;
                }
                
                appState.addClip(targetTrack, clip);
                timeline.repaint();
            }
        });
}

void MainComponent::exportAudio()
{
    // Placeholder for export
}

void MainComponent::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::black);
}

void MainComponent::resized() {
  auto bounds = getLocalBounds();
  
  // 1. Transport Bar (Top)
  transportBar.setBounds(bounds.removeFromTop(40));

  // 2. Right Pane (Agent) - Fixed width for now, say 300px
  rightPane.setBounds(bounds.removeFromRight(300));

  // 3. Left Pane (Tracks) - Fixed width for now, say 200px
  leftPane.setBounds(bounds.removeFromLeft(200));

  // 4. Timeline or Piano Roll (Remaining Center)
  if (showPianoRoll)
  {
      pianoRoll.setBounds(bounds);
  }
  else
  {
      timeline.setBounds(bounds);
  }
}
