#include "MixerComponent.h"
#include "../engine/AudioEngine.h"

//==============================================================================
class MixerChannelStrip : public juce::Component
{
public:
    MixerChannelStrip(std::shared_ptr<Track> t, AudioEngine& e) : track(t), audioEngine(e)
    {
        addAndMakeVisible(nameLabel);
        nameLabel.setText(track->name, juce::dontSendNotification);
        nameLabel.setJustificationType(juce::Justification::centred);
        
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
        
        // Instrument (MIDI only)
        if (track->type == TrackType::Midi)
        {
            instrumentButton.reset(new juce::TextButton("No Instrument"));
            addAndMakeVisible(instrumentButton.get());
            instrumentButton->onClick = [this] { showInstrumentMenu(); };
            updateInstrumentButton();
        }

        // Inserts
        for (int i = 0; i < 3; ++i)
        {
            auto* btn = new juce::TextButton("Empty");
            addAndMakeVisible(btn);
            insertButtons.add(btn);
            
            btn->onClick = [this, i] { showInsertMenu(i); };
        }
        updateInsertButtons();
    }
    
    void updateInstrumentButton()
    {
        if (instrumentButton)
        {
            if (track->instrumentPlugin && track->instrumentPlugin->instance)
            {
                instrumentButton->setButtonText(track->instrumentPlugin->instance->getName());
            }
            else
            {
                instrumentButton->setButtonText("No Instrument");
            }
        }
    }

    void showInstrumentMenu()
    {
        juce::PopupMenu menu;
        
        if (track->instrumentPlugin && track->instrumentPlugin->instance)
        {
            menu.addItem("Show Editor", [this] {
                audioEngine.showPluginWindow(track->instrumentPlugin->instance.get());
            });
            menu.addItem("Remove", [this] {
                audioEngine.closePluginWindow(track->instrumentPlugin->instance.get());
                track->instrumentPlugin = nullptr;
                updateInstrumentButton();
            });
            menu.addSeparator();
        }
        
        auto& knownPlugins = audioEngine.getKnownPluginList();
        auto types = knownPlugins.getTypes();
        
        for (const auto& desc : types)
        {
            // Filter for Instruments? (isInstrument is a property of PluginDescription)
            if (desc.isInstrument)
            {
                menu.addItem(desc.name, [this, desc] {
                    audioEngine.setInstrumentPlugin(track.get(), desc);
                    updateInstrumentButton();
                });
            }
        }
        
        menu.showMenuAsync(juce::PopupMenu::Options());
    }
    
    void updateInsertButtons()
    {
        for (int i = 0; i < insertButtons.size(); ++i)
        {
            if (i < track->insertPlugins.size() && track->insertPlugins[i] && track->insertPlugins[i]->instance)
            {
                insertButtons[i]->setButtonText(track->insertPlugins[i]->instance->getName());
            }
            else
            {
                insertButtons[i]->setButtonText("Empty");
            }
        }
    }
    
    void showInsertMenu(int slotIndex)
    {
        juce::PopupMenu menu;
        
        if (slotIndex < track->insertPlugins.size() && track->insertPlugins[slotIndex] && track->insertPlugins[slotIndex]->instance)
        {
            menu.addItem("Show Editor", [this, slotIndex] {
                audioEngine.showPluginWindow(track->insertPlugins[slotIndex]->instance.get());
            });
            menu.addItem("Remove", [this, slotIndex] {
                audioEngine.closePluginWindow(track->insertPlugins[slotIndex]->instance.get());
                track->insertPlugins[slotIndex] = nullptr;
                updateInsertButtons();
            });
            menu.addSeparator();
        }
        
        auto& knownPlugins = audioEngine.getKnownPluginList();
        auto types = knownPlugins.getTypes();
        
        for (const auto& desc : types)
        {
            menu.addItem(desc.name, [this, slotIndex, desc] {
                audioEngine.addPluginToTrack(track.get(), slotIndex, desc);
                updateInsertButtons();
            });
        }
        
        menu.showMenuAsync(juce::PopupMenu::Options());
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey.darker(0.2f));
        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds(), 1);
        
        // Meter Background
        g.setColour(juce::Colours::black);
        g.fillRect(getWidth() - 15, 30, 10, getHeight() - 60);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(4);
        
        nameLabel.setBounds(area.removeFromBottom(20));
        
        // Inserts at top
        auto insertArea = area.removeFromTop(60);
        for (auto* btn : insertButtons)
        {
            btn->setBounds(insertArea.removeFromTop(18).reduced(0, 1));
        }
        
        if (instrumentButton)
        {
            instrumentButton->setBounds(area.removeFromTop(20).reduced(0, 1));
        }
        
        area.removeFromTop(5);
        
        panSlider.setBounds(area.removeFromTop(40).reduced(5));
        
        auto btnRow = area.removeFromBottom(25);
        muteButton.setBounds(btnRow.removeFromLeft(getWidth() / 2 - 2));
        soloButton.setBounds(btnRow.removeFromRight(getWidth() / 2 - 2));
        
        // Volume and Meter
        volumeSlider.setBounds(area.removeFromLeft(area.getWidth() - 20));
    }

private:
    std::shared_ptr<Track> track;
    AudioEngine& audioEngine;
    juce::Label nameLabel;
    juce::Slider volumeSlider;
    juce::Slider panSlider;
    juce::TextButton muteButton{ "M" };
    juce::TextButton soloButton{ "S" };
    
    std::unique_ptr<juce::TextButton> instrumentButton;
    juce::OwnedArray<juce::TextButton> insertButtons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerChannelStrip)
};

//==============================================================================
MixerComponent::~MixerComponent()
{
}

//==============================================================================
MixerComponent::MixerComponent(ProjectState& state, AudioEngine& engine) 
    : projectState(state), audioEngine(engine)
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
    int w = 100;
    
    for (auto& strip : strips)
    {
        strip->setBounds(x, 0, w, getHeight());
        x += w;
    }
}

void MixerComponent::updateMixer()
{
    strips.clear();
    
    for (auto& track : projectState.tracks)
    {
        auto strip = std::make_unique<MixerChannelStrip>(track, audioEngine);
        addAndMakeVisible(strip.get());
        strips.push_back(std::move(strip));
    }
    resized();
}
