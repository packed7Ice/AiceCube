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
    synth.prepareToPlay(sampleRate, samplesPerBlockExpected);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

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
        
        // Render Synth
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
    synth.allNotesOff();
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
