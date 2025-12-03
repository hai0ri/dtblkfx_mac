#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>

class SpectrogramComponent
    : public juce::Component
    , public juce::Timer {
public:
  SpectrogramComponent(const juce::String& labelText)
      : label(labelText)
  {
    setOpaque(true);
    startTimerHz(30); // Reduced from 60Hz
  }

  ~SpectrogramComponent() override { stopTimer(); }

  void setLabel(const juce::String& newLabel)
  {
    label = newLabel;
    repaint();
  }

  void processPendingData(const float* data, int numBins)
  {
    if (image.isNull())
      return;

    // Write new column at currentX
    {
      juce::Image::BitmapData bd(image, juce::Image::BitmapData::writeOnly);
      int x = currentX;

      for (int y = 0; y < bd.height; ++y) {
        // Map y to frequency bin
        int bin = juce::jmap(y, 0, bd.height, numBins - 1, 0);
        float magnitude = data[bin];

        // Map magnitude (energy) to color
        // Data is now magnitude squared, so use 10*log10(x) or gainToDecibels(x)/2
        float db = juce::Decibels::gainToDecibels(magnitude) * 0.5f;
        float level = juce::jmap(db, -100.0f, 0.0f, 0.0f, 1.0f);
        level = juce::jlimit(0.0f, 1.0f, level);

        // Increase contrast: power curve to darken lows and brighten highs
        level = std::pow(level, 3.0f);

        juce::Colour c = juce::Colour::fromFloatRGBA(level, level, level, 1.0f);

        bd.setPixelColour(x, y, c);
      }
    }

    // Advance and wrap
    currentX = (currentX + 1) % getWidth();
    repaint();
  }

  void paint(juce::Graphics& g) override
  {
    if (image.isNull())
      return;

    int w = getWidth();
    int h = getHeight();

    // Draw in two chunks to create scrolling effect from circular buffer
    // Chunk 1: Oldest data (from currentX to end) drawn at left
    int rightChunkWidth = w - currentX;
    if (rightChunkWidth > 0) {
      g.drawImage(image,
                  0,
                  0,
                  rightChunkWidth,
                  h, // Dest
                  currentX,
                  0,
                  rightChunkWidth,
                  h // Source
      );
    }

    // Chunk 2: Newest data (from 0 to currentX) drawn at right
    if (currentX > 0) {
      g.drawImage(image,
                  rightChunkWidth,
                  0,
                  currentX,
                  h, // Dest
                  0,
                  0,
                  currentX,
                  h // Source
      );
    }

    // Draw Label
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText(label, getLocalBounds().reduced(5), juce::Justification::topRight, true);

    // Draw Grid/Overlay
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawRect(getLocalBounds(), 1);
    g.drawLine(0, h / 2.0f, (float)w, h / 2.0f);
  }

  void resized() override
  {
    image = juce::Image(juce::Image::RGB, getWidth(), getHeight(), true);
    image.clear(getLocalBounds(), juce::Colours::black);
    currentX = 0;
  }

  void timerCallback() override {}

private:
  juce::Image image;
  int currentX = 0;
  juce::String label;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrogramComponent)
};
