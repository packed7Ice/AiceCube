#include "ModernLookAndFeel.h"

ModernLookAndFeel::ModernLookAndFeel()
{
    // Define Palette
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff1e1e1e));
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d2d2d));
    setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00aaaa)); // Cyan-ish
    setColour(juce::TextButton::textColourOffId, juce::Colour(0xffdddddd));
    setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    
    setColour(juce::Slider::thumbColourId, juce::Colour(0xff00ffff));
    setColour(juce::Slider::trackColourId, juce::Colour(0xff444444));
    setColour(juce::Slider::backgroundColourId, juce::Colour(0xff222222));
    
    setColour(juce::Label::textColourId, juce::Colour(0xffdddddd));
    setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff1a1a1a));
    
    setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xff555555));
}

void ModernLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto baseColour = backgroundColour;
    
    if (shouldDrawButtonAsDown)
        baseColour = baseColour.darker(0.2f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = juce::Colour(0xff00ffff);
        
    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    g.setColour(baseColour.brighter(0.05f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void ModernLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float minSliderPos, float maxSliderPos,
                                          const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    auto trackWidth = 4.0f;
    juce::Point<float> startPoint ((float)x + (float)width * 0.5f, (float)height);
    juce::Point<float> endPoint   ((float)x + (float)width * 0.5f, (float)y);
    
    if (style == juce::Slider::LinearHorizontal)
    {
        startPoint = { (float)x, (float)y + (float)height * 0.5f };
        endPoint   = { (float)width, (float)y + (float)height * 0.5f };
    }
    
    g.setColour(slider.findColour(juce::Slider::backgroundColourId));
    g.drawLine(startPoint.x, startPoint.y, endPoint.x, endPoint.y, trackWidth);
    
    g.setColour(slider.findColour(juce::Slider::trackColourId)); // Filled part
    // Logic for filled part depending on slider mode (not fully implemented for all modes)
    
    // Thumb
    g.setColour(slider.findColour(juce::Slider::thumbColourId));
    if (style == juce::Slider::LinearHorizontal)
        g.fillEllipse(sliderPos - 5, startPoint.y - 5, 10, 10);
    else
        g.fillEllipse(startPoint.x - 5, sliderPos - 5, 10, 10);
}

void ModernLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                          const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    // Background Arc
    g.setColour(slider.findColour(juce::Slider::backgroundColourId));
    juce::Path bgArc;
    bgArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.strokePath(bgArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Value Arc
    g.setColour(slider.findColour(juce::Slider::thumbColourId));
    juce::Path valArc;
    valArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    g.strokePath(valArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void ModernLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                                      int buttonX, int buttonY, int buttonW, int buttonH,
                                      juce::ComboBox& box)
{
    auto cornerSize = 4.0f;
    juce::Rectangle<int> boxBounds (0, 0, width, height);
    
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);
    
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.0f);
    
    // Arrow
    juce::Path path;
    path.startNewSubPath((float)buttonX + (float)buttonW * 0.5f - 3.0f, (float)buttonY + (float)buttonH * 0.5f - 1.5f);
    path.lineTo((float)buttonX + (float)buttonW * 0.5f + 3.0f, (float)buttonY + (float)buttonH * 0.5f - 1.5f);
    path.lineTo((float)buttonX + (float)buttonW * 0.5f, (float)buttonY + (float)buttonH * 0.5f + 3.5f);
    path.closeSubPath();
    
    g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha((box.isEnabled() ? 0.9f : 0.2f)));
    g.fillPath(path);
}
