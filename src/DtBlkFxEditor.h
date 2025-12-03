#pragma once

#include "DtBlkFxProcessor.h"
#include "SpectrogramComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>

class DtBlkFxAudioProcessor;

#include "RetroLookAndFeel.h"

//==============================================================================
class HeaderComponent
    : public juce::Component
    , public juce::AudioProcessorValueTreeState::Listener {
public:
  HeaderComponent(juce::AudioProcessorValueTreeState& apvts);
  ~HeaderComponent() override;

  void paint(juce::Graphics& g) override;
  void resized() override;

  // Listener callback
  void parameterChanged(const juce::String& parameterID, float newValue) override;

  bool isLocked(int index) const;

private:
  juce::AudioProcessorValueTreeState& apvts;

  juce::Slider mixSlider, delaySlider, overlapSlider, fftLenSlider;
  juce::ToggleButton powerMatchButton, syncButton;

  juce::Label mixLabel, delayLabel, overlapLabel, fftLenLabel;
  juce::ComboBox delayUnitBox;
  juce::Image logo;

  // Manual handling for split params
  void updateMixParam();
  void updateDelayParam();
  void updateFFTLenParam();
  void updateOverlapParam();

  juce::ToggleButton mixLock, delayLock, fftLock, overlapLock;

  RetroLookAndFeel retroLnF;
};

//==============================================================================
class ParameterRowComponent : public juce::Component {
public:
  ParameterRowComponent(DtBlkFxAudioProcessor& p, int index);
  void paint(juce::Graphics& g) override;
  void resized() override;

  bool isLocked() const { return lockButton.getToggleState(); }

  juce::ToggleButton lockButton;
  juce::ToggleButton onOffButton;
  int lastActiveTypeId = 1; // Default to 1 (first effect) if not Off

private:
  DtBlkFxAudioProcessor& processor;
  int rowIndex;
  juce::Slider freqASlider, freqBSlider, ampSlider, valSlider, valFineSlider;
  juce::ComboBox typeBox;

  juce::Label freqALabel, freqBLabel, ampLabel, valLabel, valFineLabel;

  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqAAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqBAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ampAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> valAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;

  RetroLookAndFeel retroLnF;
};

//==============================================================================
class DtBlkFxEditor
    : public juce::AudioProcessorEditor
    , public juce::Timer {
public:
  DtBlkFxEditor(DtBlkFxAudioProcessor&);
  ~DtBlkFxEditor() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void timerCallback() override;

  // Randomization & Presets
  void startRandomization();
  void updateInterpolation();
  void savePreset();
  void loadPreset();
  void loadFactoryPreset(int index);

  struct FooterComponent : public juce::Component {
    FooterComponent(DtBlkFxEditor& editor);
    ~FooterComponent() override = default;

    void resized() override;
    void paint(juce::Graphics& g) override;

    DtBlkFxEditor& owner;
    juce::TextButton randomizeButton{"Randomize"};
    juce::Slider smoothSlider;
    juce::Label smoothLabel;
    juce::ComboBox presetBox;
  };

  struct LimiterComponent : public juce::Component {
    LimiterComponent(DtBlkFxAudioProcessor& p);
    ~LimiterComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    DtBlkFxAudioProcessor& processor;
    juce::Slider ceilingSlider, gainSlider, releaseSlider;
    juce::ToggleButton enableButton;
    juce::Label ceilingLabel, gainLabel, releaseLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ceilingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttachment;
  };

private:
  DtBlkFxAudioProcessor& audioProcessor;

  HeaderComponent header;
  FooterComponent footer;
  LimiterComponent limiter;

  SpectrogramComponent inputSpectrogram;
  SpectrogramComponent outputSpectrogram;

  // Interpolation State
  bool isInterpolating = false;
  double interpolationTime = 0.0;
  double interpolationDuration = 0.0;
  std::map<juce::String, float> startValues;
  std::map<juce::String, float> targetValues;

  juce::ComboBox inputChannelSelector;
  juce::ComboBox outputChannelSelector;

  std::vector<std::unique_ptr<ParameterRowComponent>> paramRows;

  RetroLookAndFeel retroLnF;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DtBlkFxEditor)
};
