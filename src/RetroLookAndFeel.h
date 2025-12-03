/*
  ==============================================================================

    RetroLookAndFeel.h
    Created: 1 Dec 2025
    Description: Custom LookAndFeel to match the original DtBlkFx VST2 aesthetic.

  ==============================================================================
*/

#pragma once
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

class RetroLookAndFeel : public juce::LookAndFeel_V4 {
public:
  RetroLookAndFeel()
  {
    setColour(juce::Slider::thumbColourId, juce::Colours::red.withAlpha(0.8f));
    setColour(juce::Slider::trackColourId, juce::Colours::darkgrey);
    setColour(juce::Slider::backgroundColourId, juce::Colours::black);

    setColour(juce::ComboBox::backgroundColourId, juce::Colours::black.withAlpha(0.6f));
    setColour(juce::ComboBox::outlineColourId, juce::Colours::red.withAlpha(0.5f));
    setColour(juce::ComboBox::arrowColourId, juce::Colours::red);
    setColour(juce::ComboBox::textColourId, juce::Colours::white);

    setColour(juce::TextButton::buttonColourId, juce::Colours::black.withAlpha(0.6f));
    setColour(juce::TextButton::buttonOnColourId, juce::Colours::red.withAlpha(0.5f));
    setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    setColour(juce::TextButton::textColourOnId, juce::Colours::black);

    // Global Text Color
    setColour(juce::Label::textColourId, juce::Colours::white);
    setColour(juce::GroupComponent::textColourId, juce::Colours::white);
    setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour(juce::PopupMenu::backgroundColourId, juce::Colours::black);
    setColour(juce::PopupMenu::textColourId, juce::Colours::white);

    // Remove Text Box Outline
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
  }

  void drawLinearSlider(juce::Graphics& g,
                        int x,
                        int y,
                        int width,
                        int height,
                        float sliderPos,
                        float minSliderPos,
                        float maxSliderPos,
                        const juce::Slider::SliderStyle style,
                        juce::Slider& slider) override
  {
    auto trackWidth = height * 0.3f;
    juce::Point<float> startPoint(x + width * 0.0f, y + height * 0.5f);
    juce::Point<float> endPoint(x + width * 1.0f, y + height * 0.5f);

    juce::Path backgroundTrack;
    backgroundTrack.startNewSubPath(startPoint);
    backgroundTrack.lineTo(endPoint);
    g.setColour(findColour(juce::Slider::backgroundColourId));
    g.strokePath(backgroundTrack,
                 {trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded});

    juce::Path valueTrack;
    juce::Point<float> minPoint, maxPoint, thumbPoint;

    minPoint = startPoint;
    maxPoint = {sliderPos, (float)y + height * 0.5f};

    valueTrack.startNewSubPath(minPoint);
    valueTrack.lineTo(maxPoint);

    // Retro "thermal" gradient
    juce::ColourGradient grad(
        juce::Colours::darkred.withAlpha(0.5f), minPoint, juce::Colours::red, maxPoint, false);
    g.setGradientFill(grad);
    g.strokePath(valueTrack,
                 {trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded});

    // Thumb
    auto thumbWidth = height * 0.8f;
    g.setColour(findColour(juce::Slider::thumbColourId));
    g.fillEllipse(
        juce::Rectangle<float>(static_cast<float>(trackWidth), static_cast<float>(trackWidth))
            .withCentre(maxPoint));
  }

  void drawToggleButton(juce::Graphics& g,
                        juce::ToggleButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override
  {
    auto fontSize = juce::jmin(15.0f, (float)button.getHeight() * 0.75f);
    auto tickWidth = fontSize * 1.1f;

    drawTickBox(g,
                button,
                4.0f,
                ((float)button.getHeight() - tickWidth) * 0.5f,
                tickWidth,
                tickWidth,
                button.getToggleState(),
                button.isEnabled(),
                shouldDrawButtonAsHighlighted,
                shouldDrawButtonAsDown);

    g.setColour(button.findColour(juce::ToggleButton::textColourId));
    g.setFont(fontSize);

    if (!button.isEnabled())
      g.setOpacity(0.5f);

    g.drawFittedText(button.getButtonText(),
                     button.getLocalBounds()
                         .withTrimmedLeft(juce::roundToInt(tickWidth) + 10)
                         .withTrimmedRight(2),
                     juce::Justification::centredLeft,
                     10);
  }

  void drawRotarySlider(juce::Graphics& g,
                        int x,
                        int y,
                        int width,
                        int height,
                        float sliderPos,
                        const float rotaryStartAngle,
                        const float rotaryEndAngle,
                        juce::Slider& slider) override
  {
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Fill
    g.setColour(juce::Colours::black);
    g.fillEllipse(rx, ry, rw, rw);

    // Outline
    g.setColour(findColour(juce::Slider::thumbColourId).withAlpha(0.5f));
    g.drawEllipse(rx, ry, rw, rw, 2.0f);

    // Pointer
    juce::Path p;
    auto pointerLength = radius * 0.8f;
    auto pointerThickness = 3.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(findColour(juce::Slider::thumbColourId));
    g.fillPath(p);
  }
  void drawTickBox(juce::Graphics& g,
                   juce::Component& component,
                   float x,
                   float y,
                   float w,
                   float h,
                   bool ticked,
                   bool isEnabled,
                   bool shouldDrawButtonAsHighlighted,
                   bool shouldDrawButtonAsDown) override
  {
    juce::ignoreUnused(isEnabled, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    juce::Rectangle<float> tickBounds(x, y, w, h);

    g.setColour(component.findColour(juce::ToggleButton::tickDisabledColourId));
    g.drawRect(tickBounds, 1.0f);

    if (ticked) {
      g.setColour(component.findColour(juce::ToggleButton::tickColourId));
      g.fillRect(tickBounds.reduced(2.0f));

      // Glow effect
      g.setColour(juce::Colours::cyan.withAlpha(0.4f));
      g.fillRect(tickBounds.reduced(2.0f));
    }
  }
};
