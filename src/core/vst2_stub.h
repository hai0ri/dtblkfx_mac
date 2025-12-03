#pragma once

#include <algorithm>
#include <cmath>
#include <cstring>

typedef int VstInt32;
typedef long VstIntPtr;

struct VstTimeInfo {
  double samplePos;
  double sampleRate;
  double nanoSeconds;
  double ppqPos;
  double tempo;
  double barStartPos;
  double cycleStartPos;
  double cycleEndPos;
  VstInt32 timeSigNumerator;
  VstInt32 timeSigDenominator;
  VstInt32 smpteOffset;
  VstInt32 smpteFrameRate;
  VstInt32 samplesToNextClock;
  VstInt32 flags;
};

enum VstTimeInfoFlags {
  kVstTransportChanged = 1,
  kVstTransportPlaying = 2,
  kVstTransportCycleActive = 4,
  kVstTransportRecording = 8,
  kVstAutomationWriting = 64,
  kVstAutomationReading = 128,
  kVstNanosValid = 256,
  kVstPpqPosValid = 512,
  kVstTempoValid = 1024,
  kVstBarsValid = 2048,
  kVstCyclePosValid = 4096,
  kVstTimeSigValid = 8192,
  kVstSmpteValid = 16384,
  kVstClockValid = 32768
};

enum { kPlugCategEffect = 0 };

typedef VstInt32 VstPlugCategory;

class AudioEffect {
public:
  AudioEffect() {}
  virtual ~AudioEffect() {}

  virtual void setParameter(VstInt32 index, float value) {}
  virtual float getParameter(VstInt32 index) { return 0.0f; }
  virtual void setProgram(VstInt32 program) {}
  virtual void setProgramName(char* name) {}
  virtual void getProgramName(char* name) {}
  virtual bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text)
  {
    return false;
  }

  virtual void resume() {}
  virtual void suspend() {}

  VstInt32 numPrograms = 0;
  VstInt32 curProgram = 0;
};

class AudioEffectX : public AudioEffect {
public:
  AudioEffectX(void* audioMaster, int numPrograms, int numParams)
      : AudioEffect()
  {
    this->numPrograms = numPrograms;
  }
  virtual ~AudioEffectX() {}

  virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames) = 0;
  virtual void setBlockSize(VstInt32 blockSize) {}
  virtual void setSampleRate(float sampleRate) { this->sampleRate = sampleRate; }

  virtual VstInt32 getChunk(void** data, bool isPreset = false) { return 0; }
  virtual VstInt32 setChunk(void* data, VstInt32 byteSize, bool isPreset = false) { return 0; }

  virtual bool getEffectName(char* name) { return false; }
  virtual bool getVendorString(char* text) { return false; }
  virtual bool getProductString(char* text) { return false; }
  virtual VstInt32 getVendorVersion() { return 1; }
  virtual VstPlugCategory getPlugCategory() { return kPlugCategEffect; }

  virtual VstTimeInfo* getTimeInfo(VstInt32 filter) { return &timeInfo; }

  float sampleRate = 44100.0f;
  VstTimeInfo timeInfo;
  void* editor = nullptr;

  void setNumInputs(int n) {}
  void setNumOutputs(int n) {}
  void setUniqueID(int id) {}
  void canProcessReplacing(bool b) {}
  void programsAreChunks(bool b) {}
  void isSynth(bool b) {}
  float getSampleRate() { return sampleRate; }
  void setEditor(void* e) { editor = e; }
  void setInitialDelay(int delay) {}
};

const int kVstMaxParamStrLen = 8;
const int kVstMaxProductStrLen = 64;
const int kVstMaxVendorStrLen = 64;
const int kVstMaxEffectNameLen = 32;

typedef void* audioMasterCallback;
