#ifndef Reverse_h
#define Reverse_h

#include <math.h>
#include <CircularBuffer.h>

#include "Arduino.h"
#include "HCBiquad.h"
#include "HCTapTempo.h"

#define REVERSE_BUFFERLENGTH 44100

class Reverse
{
public:

// Constructor and destructor ////////////////////////////////////////////////////////////////

  Reverse(uint16_t blockSize)
  {
    this->blockSize = blockSize;

    for(uint16_t i = 0; i < REVERSE_BUFFERLENGTH; i++)
    {
      bufferL[i] = 0;
      bufferR[i] = 0;
    }
  }
  
  ~Reverse() {}
  

// Process a block of samples ////////////////////////////////////////////////////////////////

  void processBlock(int16_t * inL, int16_t * inR, int16_t * outL, int16_t * outR)
  {  

    //wetDry = smoothWetDry->process(bypass);

    for(uint16_t i = 0; i < blockSize; i++)
    {
      bufferL[writeHead] = inL[i];
      bufferR[writeHead] = inR[i];
  
      readHead > 0 ? readHead-- : readHead = bufferLength; // Increment and wrap index
      writeHead < bufferLength ? writeHead++ : writeHead = 0;
      
      //outL[i] = inL[i] * wetDry + bufferL[readHead] * (1 - wetDry);
      //outR[i] = inR[i] * wetDry + bufferR[readHead] * (1 - wetDry);
      outL[i] = bufferL[readHead];
      outR[i] = bufferR[readHead];
    }
  }


// Setters for footswitch control ////////////////////////////////////////////////////////////////

  void fsClick()
  {
    bypass =! bypass;
    tempo = tapTempo->tickTempo();
    bufferLength = tempo * 44.1;
  }
  
  void initReverse()
  {
    bypass = true;
  }


// Getter for the led state ////////////////////////////////////////////////////////////////

  uint8_t checkLeds()
  {
    return millis() % tempo < 10 ? 2 : !bypass;
  }

private:

  TapTempo *tapTempo = new TapTempo(120);

  Biquad *smoothWetDry = new Biquad(bq_type_lowpass, 10.0 / 344, 0.5, 0);
  
// Stores the samples in PSRAM ////////////////////////////////////////////////////////////////

  int16_t bufferL[REVERSE_BUFFERLENGTH];
  int16_t bufferR[REVERSE_BUFFERLENGTH];
  

// Stores state of system ////////////////////////////////////////////////////////////////

  uint16_t blockSize;
  bool bypass = true;
  double wetDry;
  
  uint16_t tempo = 500;
  uint16_t readHead = 0;
  uint16_t writeHead = 0;
  uint16_t bufferLength = 22050;
};

#endif
