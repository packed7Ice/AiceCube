#include "MixerComponent.h"

#include "MixerComponent.h"

//==============================================================================
class MixerChannelStrip : public juce::Component
{
public:
    MixerChannelStrip(std::shared_ptr<Track> t) : track(t)
    {
        addAndMakeVisible(volumeSlider);
        volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
        volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        volumeSlider.setRange(0.0, 1.0);
        volumeSlider.setValue(track->volume, juce::dontSendNotification);
        volumeSlider.onValueChange = [this] { track->volume = (float)volumeSlider.getValue(); };
        
        addAndMakeVisible(panSlider);
        panSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        panSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        panSlider.setRange(-1.0, 1.0);
        panSlider.setValue(track->pan, juce::dontSendNotification);
        panSlider.onValueChange = [this] { track->pan = (float)panSlider.getValue(); };
        
        addAndMakeVisible(muteButton);
        muteButton.setClickingTogglesState(true);
        muteButton.setToggleState(track->mute, juce::dontSendNotification);
        muteButton.onClick = [this] { track->mute = muteButton.getToggleState(); };
        muteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
        
        addAndMakeVisible(soloButton);
        soloButton.setClickingTogglesState(true);
        soloButton.setToggleState(track->solo, juce::dontSendNotification);
        soloButton.onClick = [this] { track->solo = soloButton.getToggleState(); };
        soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow);
        
        addAndMakeVisible(nameLabel);
        nameLabel.setText(track->name, juce::dontSendNotification);
        nameLabel.setJustificationType(juce::Justification::centred);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey.darker(0.2f));
        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds(), 1);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(4);
        
        nameLabel.setBounds(area.removeFromBottom(20));
        area.removeFromBottom(5);
        
        volumeSlider.setBounds(area.removeFromBottom(120));
        area.removeFromBottom(5);
        
        auto buttonRow = area.removeFromBottom(25);
        muteButton.setBounds(buttonRow.removeFromLeft(buttonRow.getWidth() / 2).reduced(2));
        soloButton.setBounds(buttonRow.reduced(2));
        
        area.removeFromBottom(5);
        panSlider.setBounds(area.removeFromBottom(40));
    }

private:
    std::shared_ptr<Track> track;
    
    juce::Slider volumeSlider;
    juce::Slider panSlider;
    juce::TextButton muteButton{ "M" };
    juce::TextButton soloButton{ "S" };
    juce::Label nameLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerChannelStrip)
};

//==============================================================================
MixerComponent::~MixerComponent() {}

//==============================================================================
MixerComponent::MixerComponent(ProjectState& state) : projectState(state)
{
    updateMixer();
}

void MixerComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void MixerComponent::resized()
{
    int x = 0;
    int stripWidth = 80;
    
    for (auto* strip : strips)
    {
        strip->setBounds(x, 0, stripWidth, getHeight());
        x += stripWidth;
    }
}

void MixerComponent::updateMixer()
{
    strips.clear();
    
    for (auto& track : projectState.tracks)
    {
        auto* strip = new MixerChannelStrip(track);
        addAndMakeVisible(strip);
        strips.add(strip);
    }
    
    resized();
}
