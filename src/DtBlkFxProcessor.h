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

#pragma once

#include "DtBlkFx.hpp"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class DtBlkFxAudioProcessor
    : public juce::AudioProcessor
    , public juce::AudioProcessorValueTreeState::Listener {
public:
  DtBlkFxAudioProcessor();
  ~DtBlkFxAudioProcessor() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;
  //==============================================================================
  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  void parameterChanged(const juce::String& parameterID, float newValue) override;

  DtBlkFx* core = nullptr;

  juce::AudioProcessorValueTreeState apvts;
  static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

  // FIFO for spectrogram data
  // Using a simple lock-free FIFO for now. In a real app, use AbstractFifo.
  // We'll just expose a method to push data.
  void pushInputSpectrogramData(const float* data, int numBins);
  void pushOutputSpectrogramData(const float* data, int numBins);

  // Accessor for the editor to pull data
  // This is a simplification. Ideally, use a proper FIFO class.
  // For this port, we'll use a shared buffer protected by a lock (or atomic flag)
  // or just a raw pointer if we accept some tearing (visual only).
  // Let's use a simple atomic flag for "new data available"
  std::vector<float> inputSpectrogramData;
  std::atomic<bool> newInputSpectrogramDataAvailable{false};
  juce::CriticalSection inputSpectrogramLock;

  std::vector<float> outputSpectrogramData;
  std::atomic<bool> newOutputSpectrogramDataAvailable{false};
  juce::CriticalSection outputSpectrogramLock;

  // Limiter
  juce::dsp::Limiter<float> limiter;

  static constexpr auto limiterCeilingId = "limiterCeiling";
  static constexpr auto limiterGainId = "limiterGain";
  static constexpr auto limiterReleaseId = "limiterRelease";
  static constexpr auto limiterEnabledId = "limiterEnabled";

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DtBlkFxAudioProcessor)
};
