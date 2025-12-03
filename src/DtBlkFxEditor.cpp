#include "DtBlkFxEditor.h"
#include "BinaryData.h" // Generated header
#include "DtBlkFxProcessor.h"
#include "core/BlkFxParam.h"
#include "core/DtBlkFx.hpp" // For GetFxRun1_0

//==============================================================================
HeaderComponent::HeaderComponent(juce::AudioProcessorValueTreeState& apvts)
    : apvts(apvts)
{
  logo = juce::ImageCache::getFromMemory(BinaryData::dtblkfx_logo_png,
                                         BinaryData::dtblkfx_logo_pngSize);
  setLookAndFeel(&retroLnF);

  auto setupTopSlider = [&](juce::Slider& s) {
    addAndMakeVisible(s);
    s.setSliderStyle(juce::Slider::LinearHorizontal);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
  };

  // Mix & Power Match
  setupTopSlider(mixSlider);
  mixSlider.setRange(0.0, 1.0, 0.01);
  mixSlider.onValueChange = [this] { updateMixParam(); };

  // Delay
  setupTopSlider(delaySlider);
  delaySlider.setRange(0.0, 10.0, 0.01);
  delaySlider.onValueChange = [this] { updateDelayParam(); };

  // FFT Len
  setupTopSlider(fftLenSlider);
  fftLenSlider.setRange(0.0, 1.0, 0.01);
  fftLenSlider.onValueChange = [this] { updateFFTLenParam(); };

  // Overlap
  setupTopSlider(overlapSlider);
  overlapSlider.setRange(0.0, 10.0, 0.1);
  overlapSlider.onValueChange = [this] { updateOverlapParam(); };

  // Sync Button
  addAndMakeVisible(syncButton);
  syncButton.setButtonText("Sync");
  syncButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
  syncButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::green);
  syncButton.onClick = [this] { updateOverlapParam(); };

  // Labels
  auto addLabel = [this](juce::Label& l, const juce::String& text) {
    addAndMakeVisible(l);
    l.setText(text, juce::dontSendNotification);
    l.setFont(12.0f);
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, juce::Colours::white);
  };

  addLabel(mixLabel, "Dry/Wet");
  addLabel(delayLabel, "Delay");
  addLabel(fftLenLabel, "BlkLen");
  addLabel(overlapLabel, "Overlap");

  // Locks
  auto setupLock = [this](juce::ToggleButton& b) {
    addAndMakeVisible(b);
    b.setButtonText("Lock");
    b.setColour(juce::ToggleButton::textColourId, juce::Colours::white.withAlpha(0.5f));
    b.setColour(juce::ToggleButton::tickColourId, juce::Colours::red);
    b.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);
  };

  setupLock(mixLock);
  setupLock(delayLock);
  setupLock(fftLock);
  setupLock(overlapLock);

  // Register listener
  apvts.addParameterListener("param_0", this); // Mix
  apvts.addParameterListener("param_1", this); // Delay
  apvts.addParameterListener("param_2", this); // FFT Len
  apvts.addParameterListener("param_3", this); // Overlap

  // Initial update
  parameterChanged("param_0", *apvts.getRawParameterValue("param_0"));
  parameterChanged("param_1", *apvts.getRawParameterValue("param_1"));
  parameterChanged("param_2", *apvts.getRawParameterValue("param_2"));
  parameterChanged("param_3", *apvts.getRawParameterValue("param_3"));
}

HeaderComponent::~HeaderComponent()
{
  apvts.removeParameterListener("param_0", this);
  apvts.removeParameterListener("param_1", this);
  apvts.removeParameterListener("param_2", this);
  apvts.removeParameterListener("param_3", this);
  setLookAndFeel(nullptr);
}

bool HeaderComponent::isLocked(int index) const
{
  switch (index) {
    case 0:
      return mixLock.getToggleState();
    case 1:
      return delayLock.getToggleState();
    case 2:
      return fftLock.getToggleState();
    case 3:
      return overlapLock.getToggleState();
    default:
      return false;
  }
}

void HeaderComponent::parameterChanged(const juce::String& parameterID, float newValue)
{
  if (parameterID == "param_0") // Mix
  {
    bool pwrMatch = BlkFxParam::getPwrMatch(newValue) > 0.5f;
    float mixFrac = BlkFxParam::getMixBackFrac(newValue);

    juce::MessageManager::callAsync([this, pwrMatch, mixFrac] {
      mixSlider.setValue(1.0f - mixFrac, juce::dontSendNotification);
    });
  }
  else if (parameterID == "param_1") // Delay
  {
    float val = newValue * 10.0f;
    juce::MessageManager::callAsync(
        [this, val] { delaySlider.setValue(val, juce::dontSendNotification); });
  }
  else if (parameterID == "param_2") // FFT Len
  {
    juce::MessageManager::callAsync(
        [this, newValue] { fftLenSlider.setValue(newValue, juce::dontSendNotification); });
  }
  else if (parameterID == "param_3") // Overlap
  {
    float overlapPart = BlkFxParam::getOverlapPart(newValue);
    bool sync = BlkFxParam::getBlkSync(newValue);

    juce::MessageManager::callAsync([this, overlapPart, sync] {
      overlapSlider.setValue(overlapPart * 10.0f, juce::dontSendNotification);
      syncButton.setToggleState(sync, juce::dontSendNotification);
    });
  }
}

void HeaderComponent::updateMixParam()
{
  float mixFrac = (float)mixSlider.getValue();
  float newVal = BlkFxParam::getMixbackParam(1.0f - mixFrac, false);

  auto* param = apvts.getParameter("param_0");
  if (param)
    param->setValueNotifyingHost(newVal);
}

void HeaderComponent::updateDelayParam()
{
  float val = (float)delaySlider.getValue() / 10.0f;
  auto* param = apvts.getParameter("param_1");
  if (param)
    param->setValueNotifyingHost(val);
}

void HeaderComponent::updateFFTLenParam()
{
  float val = (float)fftLenSlider.getValue();
  auto* param = apvts.getParameter("param_2");
  if (param)
    param->setValueNotifyingHost(val);
}

void HeaderComponent::updateOverlapParam()
{
  float overlapPart = (float)overlapSlider.getValue() / 10.0f;
  bool sync = syncButton.getToggleState();
  float newVal = BlkFxParam::getOverlapParam(overlapPart, sync);

  auto* param = apvts.getParameter("param_3");
  if (param)
    param->setValueNotifyingHost(newVal);
}

void HeaderComponent::paint(juce::Graphics& g)
{
  g.fillAll(juce::Colours::black.withAlpha(0.2f));

  if (logo.isValid()) {
    int logoH = 60;
    int logoW = logo.getWidth() * logoH / logo.getHeight();
    int logoX = (getWidth() - logoW) / 2;
    g.drawImage(logo, logoX, 10, logoW, logoH, 0, 0, logo.getWidth(), logo.getHeight());
  }
  else {
    g.setColour(juce::Colours::red);
    g.drawRect((getWidth() - 100) / 2, 10, 100, 60, 2);
    g.drawText(
        "LOGO MISSING", (getWidth() - 100) / 2, 10, 100, 60, juce::Justification::centred, false);
  }
}

void HeaderComponent::resized()
{
  // Layout
  auto area = getLocalBounds().reduced(5);

  // Reserve space for logo
  area.removeFromTop(80); // Logo height (60) + padding (20)

  int w = area.getWidth() / 4;

  auto layoutCol = [&](int col,
                       juce::Label& label,
                       juce::Slider& slider,
                       juce::ToggleButton& lock) {
    auto r = area.withX(area.getX() + col * w).withWidth(w).reduced(5, 0);
    label.setBounds(r.removeFromTop(20));
    slider.setBounds(r.removeFromTop(30)); // Slider height

    // Center the lock button
    auto lockArea = r.removeFromTop(24);
    lock.setBounds(lockArea.withWidth(60).withX(lockArea.getX() + (lockArea.getWidth() - 60) / 2));
  };

  layoutCol(0, mixLabel, mixSlider, mixLock);
  layoutCol(1, delayLabel, delaySlider, delayLock);

  // Custom layout for Overlap to include Sync button
  {
    int col = 2;
    auto r = area.withX(area.getX() + col * w).withWidth(w).reduced(5, 0);
    overlapLabel.setBounds(r.removeFromTop(20));
    overlapSlider.setBounds(r.removeFromTop(30));

    auto bottomArea = r.removeFromTop(24);
    int buttonWidth = 50;
    int spacing = 10;
    int totalWidth = buttonWidth * 2 + spacing;
    int startX = bottomArea.getX() + (bottomArea.getWidth() - totalWidth) / 2;

    syncButton.setBounds(startX, bottomArea.getY(), buttonWidth, bottomArea.getHeight());
    overlapLock.setBounds(
        startX + buttonWidth + spacing, bottomArea.getY(), buttonWidth, bottomArea.getHeight());
  }

  layoutCol(3, fftLenLabel, fftLenSlider, fftLock);
}

//==============================================================================
ParameterRowComponent::ParameterRowComponent(DtBlkFxAudioProcessor& p, int index)
    : processor(p)
    , rowIndex(index)
{
  setLookAndFeel(&retroLnF);
  auto& apvts = p.apvts;

  int baseIndex = BlkFxParam::NUM_GLOBAL_PARAMS + index * BlkFxParam::NUM_FX_PARAMS;

  auto setupSlider = [&](juce::Slider& s, const juce::String& paramId, bool isKnob) {
    addAndMakeVisible(s);
    if (isKnob)
      s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    else
      s.setSliderStyle(juce::Slider::LinearHorizontal);

    s.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 50, 14); // Wider and slightly taller text box
  };

  setupSlider(freqASlider, "param_" + juce::String(baseIndex + 0), false); // Slider
  setupSlider(freqBSlider, "param_" + juce::String(baseIndex + 1), false); // Slider
  setupSlider(ampSlider, "param_" + juce::String(baseIndex + 2), true);    // Knob
  setupSlider(valSlider, "param_" + juce::String(baseIndex + 4), true);    // Knob

  // Fine Slider (Not attached to parameter directly)
  addAndMakeVisible(valFineSlider);
  valFineSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  valFineSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 14);
  valFineSlider.setRange(-0.1, 0.1, 0.001);
  valFineSlider.setValue(0.0);
  valFineSlider.setTextValueSuffix("");
  valFineSlider.setDoubleClickReturnValue(true, 0.0);

  valFineSlider.onValueChange = [this] {
    // Adjust main slider based on fine slider movement
    // This is a bit tricky because we want "Fine" to be an offset, but we don't want to accumulate
    // drift. Let's make it simple: Fine slider adds to the current value of the main slider when
    // moved. Actually, a better UX for "Fine" knob is: It resets to 0 on release? No. It just acts
    // as a +/- 0.1 modifier. Let's try this: We can't easily modify the main slider while dragging
    // the fine slider without feedback loops if we aren't careful. But since fine slider isn't
    // attached, it's fine.

    // Alternative: Fine slider is just a visual representation of a fine offset?
    // No, let's make it apply the delta.
    static double lastFineValue = 0.0;
    double currentFine = valFineSlider.getValue();
    double delta = currentFine - lastFineValue;
    lastFineValue = currentFine;

    if (std::abs(delta) > 0.00001) {
      valSlider.setValue(valSlider.getValue() + delta);
    }
  };

  valFineSlider.onDragStart = [this] {
    // Reset delta tracking
    // We might want to reset the knob to 0 on drag start?
    // Or just keep it relative.
    // Let's reset it to 0 on drag start to act as a "nudge" tool.
    valFineSlider.setValue(0.0, juce::dontSendNotification);
  };

  addAndMakeVisible(typeBox);
  // Populate effects dynamically
  for (int i = 0; i < g_num_fx_1_0; ++i) {
    if (auto* fx = GetFxRun1_0(i)) {
      typeBox.addItem(fx->name(), i + 1); // IDs are 1-based
    }
  }

  // Labels (Only on first row)
  if (rowIndex == 0) {
    auto setupLabel = [&](juce::Label& l, const juce::String& text) {
      addAndMakeVisible(l);
      l.setText(text, juce::dontSendNotification);
      l.setFont(10.0f);
      l.setJustificationType(juce::Justification::centred);
      l.setColour(juce::Label::textColourId, juce::Colours::white);
    };

    setupLabel(freqALabel, "Freq A");
    setupLabel(freqBLabel, "Freq B");
    setupLabel(ampLabel, "Amp");
    setupLabel(valLabel, "Val");
    setupLabel(valFineLabel, "Fine");
  }

  try {
    freqAAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "param_" + juce::String(baseIndex + 0), freqASlider);
    freqBAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "param_" + juce::String(baseIndex + 1), freqBSlider);
    ampAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "param_" + juce::String(baseIndex + 2), ampSlider);
    valAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "param_" + juce::String(baseIndex + 4), valSlider);
    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "param_" + juce::String(baseIndex + 3), typeBox);
  }
  catch (...) {
  }

  // Dynamic label logic for Amp
  ampSlider.textFromValueFunction = [this, index](double value) {
    if (processor.core) {
      float db = BlkFxParam::getEffectAmp((float)value);
      return juce::String(db, 1) + " dB";
    }
    return juce::String(value, 2);
  };

  // Freq A/B Display (0-22kHz)
  auto freqDisplay = [this](double value) {
    float hz = BlkFxParam::getHz((float)value);
    if (hz < 1000.0f)
      return juce::String(hz, 0) + " Hz";
    else
      return juce::String(hz / 1000.0f, 1) + " kHz";
  };
  freqASlider.textFromValueFunction = freqDisplay;
  freqBSlider.textFromValueFunction = freqDisplay;
  // Lock Button
  addAndMakeVisible(lockButton);
  lockButton.setButtonText("Lock");
  lockButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white.withAlpha(0.5f));
  lockButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::red);

  // On/Off Button
  addAndMakeVisible(onOffButton);
  onOffButton.setButtonText("On");
  onOffButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
  onOffButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::green);
  onOffButton.setToggleState(true, juce::dontSendNotification); // Default On

  // Handle On/Off logic
  onOffButton.onClick = [this] {
    if (onOffButton.getToggleState()) {
      // Turned ON: Restore last active type
      typeBox.setSelectedId(lastActiveTypeId, juce::sendNotification);
      onOffButton.setButtonText("On");
    }
    else {
      // Turned OFF: Store current type and set to "Off"
      int currentId = typeBox.getSelectedId();
      // Find "Off" ID
      int offId = -1;
      for (int i = 0; i < typeBox.getNumItems(); ++i) {
        if (typeBox.getItemText(i) == "Off") {
          offId = typeBox.getItemId(i);
          break;
        }
      }

      if (offId != -1 && currentId != offId) {
        lastActiveTypeId = currentId;
        typeBox.setSelectedId(offId, juce::sendNotification);
      }
      onOffButton.setButtonText("Off");
    }
  };

  // Update On/Off state when Type changes externally
  typeBox.onChange = [this] {
    if (typeBox.getText() == "Off") {
      onOffButton.setToggleState(false, juce::dontSendNotification);
      onOffButton.setButtonText("Off");
    }
    else {
      onOffButton.setToggleState(true, juce::dontSendNotification);
      onOffButton.setButtonText("On");
      lastActiveTypeId = typeBox.getSelectedId();
    }
  };
}

void ParameterRowComponent::paint(juce::Graphics& g)
{
  if (rowIndex % 2 == 0)
    g.fillAll(juce::Colours::white.withAlpha(0.05f));
}

void ParameterRowComponent::resized()
{
  auto area = getLocalBounds().reduced(2);
  int w = area.getWidth() / 6; // 6 columns

  // Column 0: Buttons (On/Lock)
  auto btnArea = area.removeFromLeft(w).reduced(2);

  // Center vertically
  int btnHeight = 24;
  int btnY = btnArea.getY() + (btnArea.getHeight() - btnHeight) / 2;

  // If row 0, adjust for label alignment (sliders have labels at top)
  // Sliders take full height, labels are top 12px.
  // Let's just center them in the available space.

  auto rowRect = btnArea.withY(btnY).withHeight(btnHeight);
  onOffButton.setBounds(rowRect.removeFromLeft(40));
  lockButton.setBounds(rowRect.removeFromRight(50));

  auto layoutItem = [&](juce::Component& ctrl, juce::Label* label) {
    auto r = area.removeFromLeft(w).reduced(2);

    // Always reserve space for label/spacer to keep alignment consistent across rows
    auto labelArea = r.removeFromTop(12);
    if (rowIndex == 0 && label) {
      label->setBounds(labelArea);
    }

    if (auto* slider = dynamic_cast<juce::Slider*>(&ctrl)) {
      if (slider->getSliderStyle() == juce::Slider::LinearHorizontal) {
        // Slider: use full remaining space so TextBoxBelow aligns with knobs
        slider->setBounds(r);
      }
      else {
        // Knob: keep square aspect ratio, but ensure it sits at the bottom or fills height
        // to align text boxes with sliders.
        int knobSize = juce::jmin(r.getWidth(), r.getHeight());
        // Center horizontally, but keep vertical fill to align text
        slider->setBounds(r.withWidth(knobSize).withX(r.getX() + (r.getWidth() - knobSize) / 2));
      }
    }
    else {
      // ComboBox (Type)
      // Visual height is approx r.getHeight() - 14.
      int dropdownHeight = 24;
      int visualHeight = r.getHeight() - 14;
      int y = r.getY() + (visualHeight - dropdownHeight) / 2;
      ctrl.setBounds(r.getX(), y, r.getWidth(), dropdownHeight);
    }
  };

  layoutItem(freqASlider, &freqALabel);
  layoutItem(freqBSlider, &freqBLabel);
  layoutItem(ampSlider, &ampLabel);
  layoutItem(typeBox, nullptr);

  // Split last column for Val and Fine
  auto valArea = area.removeFromLeft(w).reduced(2);

  // Split valArea into Coarse (70%) and Fine (30%)
  int fineWidth = valArea.getWidth() * 0.35f;
  int coarseWidth = valArea.getWidth() - fineWidth;

  auto coarseRect = valArea.removeFromLeft(coarseWidth);
  auto fineRect = valArea;

  // Layout Coarse
  auto coarseLabelArea = coarseRect.removeFromTop(12);
  if (rowIndex == 0)
    valLabel.setBounds(coarseLabelArea);

  int coarseKnobSize = juce::jmin(coarseRect.getWidth(), coarseRect.getHeight());
  valSlider.setBounds(coarseRect.withWidth(coarseKnobSize)
                          .withX(coarseRect.getX() + (coarseRect.getWidth() - coarseKnobSize) / 2));

  // Layout Fine
  auto fineLabelArea = fineRect.removeFromTop(12);
  if (rowIndex == 0)
    valFineLabel.setBounds(fineLabelArea);

  int fineKnobSize = juce::jmin(fineRect.getWidth(), fineRect.getHeight());
  valFineSlider.setBounds(fineRect.withWidth(fineKnobSize)
                              .withX(fineRect.getX() + (fineRect.getWidth() - fineKnobSize) / 2));
}

//==============================================================================
DtBlkFxEditor::LimiterComponent::LimiterComponent(DtBlkFxAudioProcessor& p)
    : processor(p)
{
  auto& apvts = p.apvts;

  auto setupSlider = [&](juce::Slider& s,
                         const juce::String& paramId,
                         const juce::String& name,
                         const juce::String& suffix) {
    addAndMakeVisible(s);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
    s.setTextValueSuffix(suffix);
  };

  setupSlider(ceilingSlider, DtBlkFxAudioProcessor::limiterCeilingId, "Ceiling", " dB");
  setupSlider(gainSlider, DtBlkFxAudioProcessor::limiterGainId, "Gain", " dB");
  setupSlider(releaseSlider, DtBlkFxAudioProcessor::limiterReleaseId, "Release", " ms");

  addAndMakeVisible(enableButton);
  enableButton.setButtonText("Limiter");
  enableButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
  enableButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::green);

  // Labels
  auto setupLabel = [&](juce::Label& l, const juce::String& text, juce::Component& target) {
    addAndMakeVisible(l);
    l.setText(text, juce::dontSendNotification);
    l.setFont(12.0f);
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, juce::Colours::white);
    l.attachToComponent(&target, false);
  };

  setupLabel(ceilingLabel, "Ceiling", ceilingSlider);
  setupLabel(gainLabel, "Gain", gainSlider);
  setupLabel(releaseLabel, "Release", releaseSlider);

  // Attachments
  ceilingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      apvts, DtBlkFxAudioProcessor::limiterCeilingId, ceilingSlider);
  gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      apvts, DtBlkFxAudioProcessor::limiterGainId, gainSlider);
  releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      apvts, DtBlkFxAudioProcessor::limiterReleaseId, releaseSlider);
  enableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
      apvts, DtBlkFxAudioProcessor::limiterEnabledId, enableButton);
}

void DtBlkFxEditor::LimiterComponent::paint(juce::Graphics& g)
{
  g.fillAll(juce::Colours::white.withAlpha(0.05f));
  g.setColour(juce::Colours::white.withAlpha(0.2f));
  g.drawRect(getLocalBounds(), 1);
}

void DtBlkFxEditor::LimiterComponent::resized()
{
  auto area = getLocalBounds().reduced(10);

  // Layout: [Enable] [Gain] [Ceiling] [Release]
  int w = area.getWidth() / 4;

  enableButton.setBounds(area.removeFromLeft(w).reduced(10, 20));
  gainSlider.setBounds(area.removeFromLeft(w).reduced(5));
  ceilingSlider.setBounds(area.removeFromLeft(w).reduced(5));
  releaseSlider.setBounds(area.removeFromLeft(w).reduced(5));
}

//==============================================================================
DtBlkFxEditor::FooterComponent::FooterComponent(DtBlkFxEditor& e)
    : owner(e)
{
  addAndMakeVisible(randomizeButton);
  randomizeButton.onClick = [&] { owner.startRandomization(); };

  addAndMakeVisible(smoothSlider);
  smoothSlider.setSliderStyle(juce::Slider::LinearHorizontal);
  smoothSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 15);
  smoothSlider.setRange(0.0, 10.0, 0.1);
  smoothSlider.setValue(1.0); // Default 1s
  smoothSlider.setTooltip("Interpolation Time (s)");

  addAndMakeVisible(smoothLabel);
  smoothLabel.setText("Smooth (s):", juce::dontSendNotification);
  smoothLabel.attachToComponent(&smoothSlider, true);

  addAndMakeVisible(presetBox);
  presetBox.addItem("Init", 1);
  presetBox.addItem("Random", 2);
  presetBox.addItem("Factory: Vocoder", 3);
  presetBox.addItem("Factory: Rhythmic", 4);
  presetBox.addSeparator();
  presetBox.addItem("Save Preset...", 100);
  presetBox.addItem("Load Preset...", 101);
  presetBox.setText("Presets");

  presetBox.onChange = [&] {
    int id = presetBox.getSelectedId();
    if (id == 1)
      owner.loadFactoryPreset(0); // Init
    else if (id == 2)
      owner.startRandomization(); // Random
    else if (id == 3)
      owner.loadFactoryPreset(1); // Vocoder
    else if (id == 4)
      owner.loadFactoryPreset(2); // Rhythmic
    else if (id == 100)
      owner.savePreset();
    else if (id == 101)
      owner.loadPreset();

    presetBox.setText("Presets"); // Reset text
  };
}

void DtBlkFxEditor::FooterComponent::paint(juce::Graphics& g)
{
  g.fillAll(juce::Colours::black);
  g.setColour(juce::Colours::white.withAlpha(0.2f));
  g.drawLine(0, 0, getWidth(), 0, 1.0f);
}

void DtBlkFxEditor::FooterComponent::resized()
{
  auto area = getLocalBounds().reduced(10);

  // Presets on the left
  presetBox.setBounds(area.removeFromLeft(120).withHeight(24));

  // Randomize on the right
  randomizeButton.setBounds(area.removeFromRight(100).withHeight(24));

  // Smooth slider in the middle, but ensure space for the attached label
  // Label is attached to the left, so we need to leave a gap.
  area.removeFromLeft(80);  // Gap for "Smooth (s):" label
  area.removeFromRight(20); // Gap before Randomize

  smoothSlider.setBounds(area.withHeight(24));
}

//==============================================================================
void DtBlkFxEditor::startRandomization()
{
  float duration = (float)footer.smoothSlider.getValue();

  startValues.clear();
  targetValues.clear();

  auto& params = audioProcessor.getParameters();
  juce::Random rng;

  for (auto* p : params) {
    if (auto* param = dynamic_cast<juce::AudioProcessorParameterWithID*>(p)) {
      // Skip Limiter parameters
      if (param->paramID.startsWith("limiter")) {
        continue;
      }

      // Check locks for global parameters (0-3)
      // Param IDs are "param_0", "param_1", etc.
      // We can parse the ID or check index if available.
      // Let's assume paramID format "param_X"
      int paramIndex = param->paramID.fromFirstOccurrenceOf("param_", false, false).getIntValue();

      if (paramIndex < 4 && header.isLocked(paramIndex)) {
        continue; // Skip locked global params
      }

      // Check locks for FX rows
      if (paramIndex >= 4) {
        int fxIndex = (paramIndex - 4) / 5; // 5 params per row
        if (fxIndex >= 0 && fxIndex < paramRows.size() && paramRows[fxIndex]->isLocked()) {
          continue; // Skip locked FX row
        }
      }

      startValues[param->paramID] = param->getValue();
      targetValues[param->paramID] = rng.nextFloat();
    }
  }

  if (duration > 0.0f) {
    isInterpolating = true;
    interpolationTime = 0.0;
    interpolationDuration = duration;
  }
  else {
    // Instant
    for (auto const& [id, val] : targetValues) {
      if (auto* param = audioProcessor.apvts.getParameter(id)) {
        param->setValueNotifyingHost(val);
      }
    }
  }
}

void DtBlkFxEditor::updateInterpolation()
{
  if (!isInterpolating)
    return;

  interpolationTime += 1.0 / 60.0;
  float progress = (float)(interpolationTime / interpolationDuration);

  if (progress >= 1.0f) {
    progress = 1.0f;
    isInterpolating = false;
  }

  for (auto const& [id, target] : targetValues) {
    if (auto* param = audioProcessor.apvts.getParameter(id)) {
      float start = startValues[id];
      float current = start + (target - start) * progress;
      param->setValueNotifyingHost(current);
    }
  }
}

void DtBlkFxEditor::savePreset()
{
  auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                  .getChildFile("DtBlkFx_Presets")
                  .getNonexistentChildFile("Preset", ".xml");

  juce::FileChooser fc("Save Preset", file, "*.xml");
  if (fc.browseForFileToSave(true)) {
    auto xml = audioProcessor.apvts.copyState().createXml();
    xml->writeTo(fc.getResult());
  }
}

void DtBlkFxEditor::loadPreset()
{
  juce::FileChooser fc("Load Preset",
                       juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                           .getChildFile("DtBlkFx_Presets"),
                       "*.xml");

  if (fc.browseForFileToOpen()) {
    auto xml = juce::XmlDocument::parse(fc.getResult());
    if (xml) {
      audioProcessor.apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
  }
}

void DtBlkFxEditor::loadFactoryPreset(int index)
{
  startValues.clear();
  targetValues.clear();

  auto setParam = [&](int id, float val) {
    juce::String paramID = "param_" + juce::String(id);
    targetValues[paramID] = val;
    if (auto* p = audioProcessor.apvts.getParameter(paramID))
      startValues[paramID] = p->getValue();
  };

  // Reset all first
  for (int i = 0; i < BlkFxParam::TOTAL_NUM; ++i) {
    setParam(i, 0.0f); // Default 0
  }

  // Apply specific settings
  if (index == 0) {    // Init
    setParam(0, 0.0f); // Mix Dry
    setParam(2, 0.5f); // FFT
    setParam(3, 0.5f); // Overlap
  }
  else if (index == 1) { // Vocoder-ish
    setParam(0, 1.0f);   // Wet
    setParam(2, 0.7f);   // FFT
  }

  // Trigger interpolation (short)
  isInterpolating = true;
  interpolationTime = 0.0;
  interpolationDuration = 0.5; // 0.5s transition for presets
}

//==============================================================================
DtBlkFxEditor::DtBlkFxEditor(DtBlkFxAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , audioProcessor(p)
    , header(p.apvts)
    , inputSpectrogram("Input Left")
    , outputSpectrogram("Output Left")
    , footer(*this)
    , limiter(p)
{
  setLookAndFeel(&retroLnF);

  addAndMakeVisible(header);
  addAndMakeVisible(inputSpectrogram);
  addAndMakeVisible(outputSpectrogram);
  addAndMakeVisible(footer);
  addAndMakeVisible(limiter);

  // Channel Selectors Removed
  inputSpectrogram.setLabel("Input L+R");
  outputSpectrogram.setLabel("Output L+R");

  for (int i = 0; i < 8; ++i) {
    auto row = std::make_unique<ParameterRowComponent>(p, i);
    addAndMakeVisible(*row);
    paramRows.push_back(std::move(row));
  }

  // Increased height to fit all rows + limiter
  setSize(600, 1040); // 960 + 80 for limiter
  startTimerHz(60);
}

DtBlkFxEditor::~DtBlkFxEditor()
{
  stopTimer();
  paramRows.clear();
  setLookAndFeel(nullptr);
}

void DtBlkFxEditor::timerCallback()
{
  if (audioProcessor.newInputSpectrogramDataAvailable) {
    juce::ScopedLock lock(audioProcessor.inputSpectrogramLock);
    inputSpectrogram.processPendingData(audioProcessor.inputSpectrogramData.data(),
                                        (int)audioProcessor.inputSpectrogramData.size());
    audioProcessor.newInputSpectrogramDataAvailable = false;
  }

  if (audioProcessor.newOutputSpectrogramDataAvailable) {
    juce::ScopedLock lock(audioProcessor.outputSpectrogramLock);
    outputSpectrogram.processPendingData(audioProcessor.outputSpectrogramData.data(),
                                         (int)audioProcessor.outputSpectrogramData.size());
    audioProcessor.newOutputSpectrogramDataAvailable = false;
  }

  updateInterpolation();
}

void DtBlkFxEditor::paint(juce::Graphics& g)
{
  g.fillAll(juce::Colours::black);
}

void DtBlkFxEditor::resized()
{
  auto area = getLocalBounds();

  header.setBounds(area.removeFromTop(165)); // Increased header height for logo + locks

  // Limiter at the very bottom
  limiter.setBounds(area.removeFromBottom(80));

  footer.setBounds(area.removeFromBottom(60));

  int specHeight = 80;                                     // Shortened spectrograms
  auto specArea = area.removeFromTop(specHeight * 2 + 10); // Reduced padding

  // inputChannelSelector removed
  inputSpectrogram.setBounds(specArea.removeFromTop(specHeight));
  specArea.removeFromTop(10); // Spacing

  // outputChannelSelector removed
  outputSpectrogram.setBounds(specArea.removeFromTop(specHeight));

  int rowHeight = 70; // Reduced row height
  for (auto& row : paramRows) {
    row->setBounds(area.removeFromTop(rowHeight));
  }
}
