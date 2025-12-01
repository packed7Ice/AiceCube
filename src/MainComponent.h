#pragma once
#include <juce_gui_basics/juce_gui_basics.h> // Component, Graphics など

#include "AgentPanel.h"
#include "ApiClient.h"
#include "PianoRollComponent.h"
#include "SimpleSynth.h"

class MainComponent : public juce::AudioAppComponent {
public:
  MainComponent();
  ~MainComponent() override;

  void paint(juce::Graphics &g) override;
  void resized() override;

  // AudioAppComponent overrides
  void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
  void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
  void releaseResources() override;

private:
  AgentPanel agentPanel;
  ApiClient apiClient;
  PianoRollComponent pianoRoll;
  SimpleSynth synth;
  
  // Simple playback state
  bool isPlaying = false;
  double currentPlayTime = 0.0;
  double sampleRate = 44100.0;

  void playSequence();
  void stopSequence();
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
