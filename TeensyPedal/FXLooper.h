#ifndef Looper_h
#define Looper_h

#include "Arduino.h"

#define LOOPER_MAX_SIZE 441000

class Looper
{
public:


// Constructor and destructor ////////////////////////////////////////////////////////////////

  Looper(uint16_t blockSize, bool killDry)
  {
    this->blockSize = blockSize;
    this->killDry = killDry;

    debounce = millis();

    for(uint32_t i = 0; i < LOOPER_MAX_SIZE; i++)
    {
      bufferL[i] = 0;
      bufferR[i] = 0;
    }
  }
  ~Looper() {}
  

// Process a block of samples ////////////////////////////////////////////////////////////////

  void processBlock(int16_t * inL, int16_t * inR, int16_t * outL, int16_t * outR)
  {
    for(uint16_t i = 0; i < blockSize; i++)
    {
      if(recording)
      {
        bufferL[bufStep] = constrain((bufferL[bufStep] * playing) + inL[i], -32767, 32766);
        bufferR[bufStep] = constrain((bufferR[bufStep] * playing) + inR[i], -32767, 32766);
        checkLength();
      }

      bufStep < bufLen ? bufStep++ : bufStep = 0; // Increment and wrap index

      killDry ? dryOut = !playing : dryOut = true;
    
      outL[i] = constrain((inL[i] * dryOut) + (bufferL[bufStep] * playing), -32767, 32766);
      outR[i] = constrain((inR[i] * dryOut) + (bufferR[bufStep] * playing), -32767, 32766);
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
    if(!blockRec && millis() > (debounce + 25))
    {
      recording = true;
      blockRec = true;
    }
    lastHold = millis();
  }
  
  void fsStop()
  {
    recording = false;
    playing = true;
    blockRec = false;
    debounce = millis();
    fades(100);
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
      bufLen < LOOPER_MAX_SIZE ? bufLen++ : playing = true;
    }
  }

  void fades(uint16_t fadeLength)
  {
    for(uint16_t i = 0; i < fadeLength; i++)
    {
      double gain = double(i) / fadeLength;
      
      bufferL[i] = bufferL[i] * gain;
      bufferR[i] = bufferR[i] * gain;

      bufferL[bufLen - i] = bufferL[bufLen - i] * gain;
      bufferR[bufLen - i] = bufferR[bufLen - i] * gain;
    }
  }

  
// Stores the samples in PSRAM ////////////////////////////////////////////////////////////////

  int16_t bufferL[LOOPER_MAX_SIZE];
  int16_t bufferR[LOOPER_MAX_SIZE];

  
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
  uint32_t lastHold;
  uint16_t blockSize;
};

#endif
