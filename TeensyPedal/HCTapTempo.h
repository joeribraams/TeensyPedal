#ifndef TapTempo_h
#define TapTempo_h

#include <math.h>
#include <CircularBuffer.h>

class TapTempo
{
public:

  TapTempo(uint16_t BPM)
  {
    this->BPM = BPM;
    timeInterval = 60000 / BPM;
    lastTap = millis();
  }
 
  ~TapTempo() {}
  
  uint16_t tickTempo()
  {
    uint16_t newInterval = millis() - lastTap;

    if(newInterval < 1500)
    {
      timeInterval = newInterval;
    }  

    lastTap = millis();
    return timeInterval;
  }
    
private:

  uint32_t lastTap;
  uint16_t timeInterval;
  uint16_t BPM;
};

#endif // TapTempo_h
