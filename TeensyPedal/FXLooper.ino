#ifndef Looper_h
#define Looper_h

#include "Arduino.h"

class Looper
{
public:


// Constructor and destructor ////////////////////////////////////////////////////////////////

  Looper(uint16_t blockSize, uint32_t maxSize, bool killDry)
  {
    this->blockSize = blockSize;
    this->maxSize = maxSize;
    this->killDry = killDry;
  }
  ~Looper() {}
  

// Process a block of samples ////////////////////////////////////////////////////////////////

  void processBlock(int16_t * inL, int16_t * inR, int16_t * outL, int16_t * outR)
  {
    for(uint16_t i = 0; i < blockSize; i++)
    {
      if(recording)
      {
        bufferL[bufStep] = (bufferL[bufStep] * playing) + inL[i];
        bufferR[bufStep] = (bufferR[bufStep] * playing) + inR[i];
        checkLength();
      }

      bufStep <= bufLen ? bufStep++ : bufStep = 0; // Increment and wrap index

      killDry ? dryOut = !playing : dryOut = true;
    
      outL[i] = (inL[i] * !playing) + (bufferL[bufStep] * playing);
      outR[i] = (inL[i] * !playing) + (bufferR[bufStep] * playing);
    }
  }


// Setters for footswitch control ////////////////////////////////////////////////////////////////

  void fsClick()
  {
    if(playing)
    {
      initLooper();
    }
    else // A short tap when not playing can also start the loop
    {
      fsStop();
    }
  }
  
  void fsHold()
  {
    if(!blockRec && millis() > (debounce + 50))
    {
      recording = true;
      blockRec = true;
    }
  }
  
  void fsStop()
  {
    recording = false;
    playing = true;
    blockRec = false;
    debounce = millis();
  }

  void initLooper()
  {
    playing = false;
    recording = false;
    blockRec = false;
    debounce = millis();
    bufStep = 0;
    bufLen = 0;
  }


// Getter for the led state ////////////////////////////////////////////////////////////////

  uint8_t checkLeds()
  {
    return recording ? 2 : playing;
  }

private:


// To prevent buffer overflow ////////////////////////////////////////////////////////////////

  void checkLength()
  {
    if(!playing)
    {
      bufLen < maxSize ? bufLen++ : playing = true;
    }
  }

  
// Stores the samples in PSRAM ////////////////////////////////////////////////////////////////

  EXTMEM int bufferL[maxSize];
  EXTMEM int bufferR[maxSize];

  
// Place in and length of loop ////////////////////////////////////////////////////////////////

  uint32_t bufStep = 0;
  uint32_t bufLen = 0;


// Stores state of system ////////////////////////////////////////////////////////////////

  bool recording = false;
  bool playing = false;
  bool blockRec = false;
  bool killDry;
  bool dryOut = true;
  uint32_t debounce;
  uint16_t blockSize;
  uint32_t maxSize;
};

#endif
