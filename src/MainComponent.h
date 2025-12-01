#pragma once
#include <juce_gui_basics/juce_gui_basics.h> // Component, Graphics など

#include "AppState.h"
#include "TransportBarComponent.h"
#include "LeftPaneComponent.h"
#include "TimelineComponent.h"
#include "BottomPaneComponent.h"
#include "ApiClient.h"
#include "SimpleSynth.h"

#include "CommandExecutor.h"

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
  AppState appState;
  
  TransportBarComponent transportBar{ appState };
  LeftPaneComponent leftPane{ appState };
  TimelineComponent timeline{ appState };
  BottomPaneComponent bottomPane;
  
  ApiClient apiClient;
  SimpleSynth synth;
  CommandExecutor commandExecutor{ appState, apiClient };
  
  double sampleRate = 44100.0;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
