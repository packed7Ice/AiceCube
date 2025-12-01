#include "AppState.h"
#include "TransportBarComponent.h"
#include "LeftPaneComponent.h"
#include "TimelineComponent.h"
#include "RightPaneComponent.h"
#include "ApiClient.h"
#include "SimpleSynth.h"

#include "CommandExecutor.h"

#include "PianoRollComponent.h"

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
  RightPaneComponent rightPane;
  PianoRollComponent pianoRoll; // New
  
  ApiClient apiClient;
  SimpleSynth synth;
  CommandExecutor commandExecutor{ appState, apiClient };
  
  double sampleRate = 44100.0;
  bool showPianoRoll = false; // View state

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
