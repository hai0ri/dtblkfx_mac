/*
 * See LICENSE.md for copyright and licensing information.
 *
 * This file is part of DtBlkFx.
 *
 * DtBlkFx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DtBlkFx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DtBlkFx.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "DtBlkFxProcessor.h"
#include "DtBlkFxEditor.h"
#include "rfftw_float.h"

DtBlkFxAudioProcessor::DtBlkFxAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#  if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#  endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         )
    , apvts(*this, nullptr, "Parameters", createParameterLayout())
{
  static bool initialized = false;
  if (!initialized) {
    CreateFFTWfPlans();
    initialized = true;
  }

  core = new DtBlkFx(nullptr);
  core->setSampleRate(getSampleRate());
  core->setBlockSize(getBlockSize());

  core->inputSpectrogramCallback = [this](const float* data, int numBins) {
    pushInputSpectrogramData(data, numBins);
  };
  core->outputSpectrogramCallback = [this](const float* data, int numBins) {
    pushOutputSpectrogramData(data, numBins);
  };

  // Add listeners
  for (int i = 0; i < BlkFxParam::TOTAL_NUM; ++i) {
    apvts.addParameterListener("param_" + juce::String(i), this);
    // Sync initial values
    core->setParameter(i, apvts.getRawParameterValue("param_" + juce::String(i))->load());
  }
}

juce::AudioProcessorValueTreeState::ParameterLayout DtBlkFxAudioProcessor::createParameterLayout()
{
  juce::AudioProcessorValueTreeState::ParameterLayout layout;

  for (int i = 0; i < BlkFxParam::TOTAL_NUM; ++i) {
    BlkFxParam::SplitParamNum p(i);
    juce::String name;
    float defaultValue = 0.0f;
    if (p.glob_param >= 0) {
      switch (p.glob_param) {
        case BlkFxParam::MIX_BACK:
          name = "Mix Back";
          defaultValue = 0.0f;
          break;
        case BlkFxParam::DELAY:
          name = "Delay";
          defaultValue = 0.0f;
          break;
        case BlkFxParam::FFT_LEN:
          name = "FFT Length";
          defaultValue = 0.5f;
          break;
        case BlkFxParam::OVERLAP:
          name = "Overlap";
          defaultValue = 0.5f;
          break;
        default:
          name = "Global " + juce::String(p.glob_param);
          break;
      }
    }
    else {
      juce::String fxName = "FX " + juce::String(p.fx_set + 1) + " ";
      switch (p.fx_param) {
        case BlkFxParam::FX_FREQ_A:
          name = fxName + "Freq A";
          break;
        case BlkFxParam::FX_FREQ_B:
          name = fxName + "Freq B";
          break;
        case BlkFxParam::FX_AMP:
          name = fxName + "Amp";
          defaultValue = 0.6f; // 0dB
          break;
        case BlkFxParam::FX_TYPE:
          name = fxName + "Type";
          break;
        case BlkFxParam::FX_VAL:
          name = fxName + "Value";
          break;
        default:
          name = fxName + "Param " + juce::String(p.fx_param);
          break;
      }
    }

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "param_" + juce::String(i), name, 0.0f, 1.0f, defaultValue));
  }

  // Limiter Parameters
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      limiterCeilingId,
      "Limiter Ceiling",
      juce::NormalisableRange<float>(-24.0f, 0.0f, 0.1f),
      -0.1f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      limiterGainId, "Limiter Gain", juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 0.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      limiterReleaseId,
      "Limiter Release",
      juce::NormalisableRange<float>(0.1f, 1000.0f, 0.1f, 0.5f),
      50.0f));
  layout.add(std::make_unique<juce::AudioParameterBool>(limiterEnabledId, "Limiter Enabled", true));

  return layout;
}

void DtBlkFxAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
  if (core) {
    if (parameterID.startsWith("param_")) {
      int index = parameterID.substring(6).getIntValue();
      core->setParameter(index, newValue);
    }
  }
}

DtBlkFxAudioProcessor::~DtBlkFxAudioProcessor()
{
  if (core) {
    delete core;
    core = nullptr;
  }
}

const juce::String DtBlkFxAudioProcessor::getName() const
{
  return JucePlugin_Name;
}

bool DtBlkFxAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool DtBlkFxAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool DtBlkFxAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double DtBlkFxAudioProcessor::getTailLengthSeconds() const
{
  return 0.0;
}

int DtBlkFxAudioProcessor::getNumPrograms()
{
  return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
            // so this should be at least 1, even if you're not really implementing programs.
}

int DtBlkFxAudioProcessor::getCurrentProgram()
{
  return 0;
}

void DtBlkFxAudioProcessor::setCurrentProgram(int index)
{
  juce::ignoreUnused(index);
}

const juce::String DtBlkFxAudioProcessor::getProgramName(int index)
{
  juce::ignoreUnused(index);
  return {};
}

void DtBlkFxAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
  juce::ignoreUnused(index, newName);
}

void DtBlkFxAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  if (core) {
    core->setSampleRate(sampleRate);
    core->setBlockSize(samplesPerBlock);
    core->resume();
  }

  juce::dsp::ProcessSpec spec;
  spec.sampleRate = sampleRate;
  spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
  spec.numChannels = (juce::uint32)getTotalNumOutputChannels();

  limiter.prepare(spec);
  limiter.reset();
}

void DtBlkFxAudioProcessor::releaseResources()
{
  if (core) {
    core->suspend();
  }
}

bool DtBlkFxAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#  if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#  endif

  return true;
#endif
}

void DtBlkFxAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer& midiMessages)
{
  juce::ignoreUnused(midiMessages);

  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  // In case we have more outputs than inputs, this code clears any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  // This is here to avoid people getting screaming feedback
  // when they first compile a plugin, but obviously you don't need to keep
  // this code if your algorithm always overwrites all the output channels.
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  if (core) {
    core->processReplacing(
        buffer.getArrayOfWritePointers(), buffer.getArrayOfWritePointers(), buffer.getNumSamples());
  }

  // Output Limiter
  bool limiterEnabled = *apvts.getRawParameterValue(limiterEnabledId) > 0.5f;
  if (limiterEnabled) {
    float ceiling = *apvts.getRawParameterValue(limiterCeilingId);
    float gain = *apvts.getRawParameterValue(limiterGainId);
    float release = *apvts.getRawParameterValue(limiterReleaseId);

    // Apply Input Gain
    if (gain > 0.0f) {
      buffer.applyGain(juce::Decibels::decibelsToGain(gain));
    }

    limiter.setThreshold(ceiling);
    limiter.setRelease(release);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    limiter.process(context);
  }
}
//==============================================================================
bool DtBlkFxAudioProcessor::hasEditor() const
{
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DtBlkFxAudioProcessor::createEditor()
{
  return new DtBlkFxEditor(*this);
}

//==============================================================================
void DtBlkFxAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries
  auto state = apvts.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void DtBlkFxAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
  // You should use this method to restore your parameters from this memory block,
  // whose contents will have been created by the getStateInformation() call.
  std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(apvts.state.getType()))
      apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void DtBlkFxAudioProcessor::pushInputSpectrogramData(const float* data, int numBins)
{
  juce::ScopedLock lock(inputSpectrogramLock);
  if (inputSpectrogramData.size() != numBins)
    inputSpectrogramData.resize(numBins);

  std::memcpy(inputSpectrogramData.data(), data, numBins * sizeof(float));
  newInputSpectrogramDataAvailable = true;
}

void DtBlkFxAudioProcessor::pushOutputSpectrogramData(const float* data, int numBins)
{
  juce::ScopedLock lock(outputSpectrogramLock);
  if (outputSpectrogramData.size() != numBins)
    outputSpectrogramData.resize(numBins);

  std::memcpy(outputSpectrogramData.data(), data, numBins * sizeof(float));
  newOutputSpectrogramDataAvailable = true;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
  return new DtBlkFxAudioProcessor();
}
