#ifndef Feedbacker_h
#define Feedbacker_h

#include <math.h>
#include <CircularBuffer.h>

#include "Arduino.h"
#include "HCBiquad.h"
#include "HCTapTempo.h"

#define FEEDBACKER_BUFFERLENGTH 44100

class Feedbacker
{
public:

// Constructor and destructor ////////////////////////////////////////////////////////////////

  Feedbacker(uint16_t blockSize, double feedback, double wetLevel, double thresh)
  {
    this->blockSize = blockSize;
    this->feedback = feedback;
    this->wetLevel = wetLevel * 32767;
    this->thresh = thresh * 32767;

    for(uint16_t i = 0; i < FEEDBACKER_BUFFERLENGTH; i++)
    {
      buffer.push(0);
    }
  }
  ~Feedbacker() {}
  

// Process a block of samples ////////////////////////////////////////////////////////////////

  void processBlock(int16_t * inL, int16_t * inR, int16_t * outL, int16_t * outR)
  {  
    blockAvg = 0;

    dryInput = inSmooth->process(!bypass);
    bypassFeedback = fbSmooth->process((bypass ? 0.4 : feedback) * !muteFb);   
    
    for(uint16_t i = 0; i < blockSize; i++)
    {
      blockAvg += abs(inL[i]) + abs(inR[i]);
      
      buffer.push((double(inL[i]) + double(inR[i])) / 16384 * dryInput + fb * bypassFeedback);

      fb = saturate(lowPass->process(highPass->process(saturate(buffer.first()))));

      outL[i] = inL[i] + fb * wetLevel;
      outR[i] = inR[i] + fb * wetLevel;
    }

    if(blockAvg / (blockSize * 2) > thresh && !muteFb)
    {
      muteFb = true;
      lastPeak = millis();
    }

    if(millis() > lastPeak + FEEDBACKER_BUFFERLENGTH / 44.1)
    {
      muteFb = false;
    }
  }


// Setters for footswitch control ////////////////////////////////////////////////////////////////

  void fsClick()
  {
    bypass =! bypass;
    tempo = tapTempo->tickTempo();
  }
  
  void initFeedbacker()
  {
    dryInput = 0;
    bypassFeedback = 0.5;
    fb = 0;
    muteFb = false;
    lastPeak = 0;
    peakSample = 0;
    bypass = true;
  }


// Getter for the led state ////////////////////////////////////////////////////////////////

  uint8_t checkLeds()
  {
    return millis() % tempo < 10 ? 2 : !bypass;
  }

private:

  double saturate(double in)
  {
    return atan(atan(atan(in)));
  }
  
// Stores the samples in PSRAM ////////////////////////////////////////////////////////////////

  CircularBuffer<double,FEEDBACKER_BUFFERLENGTH> buffer;
  int32_t blockAvg = 0;
  
  Biquad *lowPass = new Biquad(bq_type_lowpass, 3000.0 / 44100, 0.5, 0);
  Biquad *highPass = new Biquad(bq_type_highpass, 100.0 / 44100, 0.5, 0);
  
  Biquad *inSmooth = new Biquad(bq_type_lowpass, 10.0 / 344, 0.5, 0);
  Biquad *fbSmooth = new Biquad(bq_type_lowpass, 10.0 / 344, 0.5, 0);

  TapTempo *tapTempo = new TapTempo(120);

// Stores state of system ////////////////////////////////////////////////////////////////

  uint16_t blockSize;
  double dryInput = 0;
  double feedback;
  double bypassFeedback = 0.5;
  double fb = 0;
  uint16_t wetLevel;
  uint16_t thresh;
  bool muteFb = false;
  uint32_t lastPeak = 0;
  uint16_t peakSample = 0;
  bool bypass = true;

  uint16_t tempo = 500;
};

#endif
