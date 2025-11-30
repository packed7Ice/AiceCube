#include "MainComponent.h"

MainComponent::MainComponent() { setSize(800, 600); }

void MainComponent::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::black);

  g.setColour(juce::Colours::cyan);
  g.setFont(24.0f);
  g.drawText("AiceCube - Agent-based Music Tool (WIP)", getLocalBounds(),
             juce::Justification::centred, true);
}

void MainComponent::resized() {
  // 後で子コンポーネントを配置するときに使う
}
