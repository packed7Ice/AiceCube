#include "MainComponent.h"

#include "AgentLogic.h"

MainComponent::MainComponent() {
  setSize(800, 600);
  
  addAndMakeVisible(pianoRoll);
  addAndMakeVisible(agentPanel);

  // Setup Piano Roll Play Button
  pianoRoll.onPlayButtonClicked = [this] {
      if (isPlaying) stopSequence();
      else playSequence();
  };

  agentPanel.onCommandEntered = [this](const juce::String& command) {
      auto results = AgentLogic::interpretInstruction(command);
      if (results.isEmpty())
      {
          agentPanel.logMessage("No command recognized.");
      }
      else
      {
          for (const auto& cmd : results)
          {
              agentPanel.logMessage("Parsed: " + cmd.toString());
              
              if (cmd.type == AgentCommand::Type::AddMelody)
              {
                  agentPanel.logMessage("Sending request to server...");
                  apiClient.generateMelody(cmd, [this](const Sequence& seq) {
                      agentPanel.logMessage("Received " + juce::String(seq.notes.size()) + " notes from server.");
                      pianoRoll.setSequence(seq);
                  });
              }
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

    if (isPlaying)
    {
        // Very basic sequencer logic in audio thread (not ideal but works for simple demo)
        // Check for notes to trigger
        const auto& seq = pianoRoll.getSequence();
        double samplesPerBeat = (60.0 / 120.0) * sampleRate; // Assume 120 BPM
        
        int numSamples = bufferToFill.numSamples;
        
        for (int i = 0; i < numSamples; ++i)
        {
            double timeInBeats = currentPlayTime / samplesPerBeat;
            double nextTimeInBeats = (currentPlayTime + 1.0) / samplesPerBeat;

            // Check note starts
            for (const auto& note : seq.notes)
            {
                if (note.start >= timeInBeats && note.start < nextTimeInBeats)
                {
                    synth.noteOn(note.pitch, note.velocity / 127.0f);
                }
                // Check note ends (simple duration check)
                if ((note.start + note.duration) >= timeInBeats && (note.start + note.duration) < nextTimeInBeats)
                {
                    synth.noteOff(note.pitch);
                }
            }
            
            currentPlayTime += 1.0;
        }
        
        // Render Synth
        juce::AudioBuffer<float> synthBuffer(bufferToFill.buffer->getArrayOfWritePointers(), 
                                             bufferToFill.buffer->getNumChannels(), 
                                             bufferToFill.startSample, 
                                             bufferToFill.numSamples);
        synth.renderNextBlock(synthBuffer, 0, numSamples);
    }
}

void MainComponent::releaseResources()
{
    synth.allNotesOff();
}

void MainComponent::playSequence()
{
    currentPlayTime = 0.0;
    isPlaying = true;
}

void MainComponent::stopSequence()
{
    isPlaying = false;
    synth.allNotesOff();
}

void MainComponent::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::darkgrey);
  
  // Header
  g.setColour(juce::Colours::white);
  g.setFont(20.0f);
  g.drawText("AiceCube", getLocalBounds().removeFromTop(20), juce::Justification::centred, true);
}

void MainComponent::resized() {
  auto bounds = getLocalBounds();
  
  // Reserve space for Header
  bounds.removeFromTop(20);

  // Agent Panel at bottom
  auto agentPanelBounds = bounds.removeFromBottom(150);
  agentPanel.setBounds(agentPanelBounds);

  // Piano Roll takes remaining space
  pianoRoll.setBounds(bounds);
}
