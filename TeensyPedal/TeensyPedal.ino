// Includes ////////////////////////////////////////////////////////////////

#include <MIDI.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <OneButton.h>

#include "FxLooper.h"


// Pin Defines ////////////////////////////////////////////////////////////////

#define RSWA0 2 // Rotary Switch A
#define RSWA1 3
#define RSWA2 4
#define RSWB0 15 // Rotary Switch B
#define RSWB1 16
#define RSWB2 17
#define FSA 10  // Footswitches
#define FSB 9
#define LAR 12  // Leds, red and blue respectively
#define LAB 11
#define LBR 6
#define LBB 5


// Effect Defines ////////////////////////////////////////////////////////////////

//#define FX0A
//#define FX1A
//#define FX2A
//#define FX3A
//#define FX4A
//#define FX5A
//#define FX6A
//#define FX7A

//#define FX0B
//#define FX1B
//#define FX2B
//#define FX3B
//#define FX4B
//#define FX5B
//#define FX6B
//#define FX7B


// Variables ////////////////////////////////////////////////////////////////

const uint16_t blockSize=AUDIO_BLOCK_SAMPLES; // Don't know why but it works

int16_t * inL;
int16_t * inR;
int16_t * midL; // For passing samples between the two sides
int16_t * midR;
int16_t * outL;
int16_t * outR;

uint8_t presetA = 0;
uint8_t presetB = 0;
uint8_t oldPresetA = 0;
uint8_t oldPresetB = 0;


// Init Classes ////////////////////////////////////////////////////////////////

OneButton fsa = OneButton(FSA, true, false);
OneButton fsb = OneButton(FSB, true, false);

AudioControlWM8731   wm8731; // Created by Teensy audio tool
AudioInputI2S        I2SIN;
AudioOutputI2S       I2SOUT;
AudioRecordQueue     INL;
AudioRecordQueue     INR;
AudioPlayQueue       OUTL;
AudioPlayQueue       OUTR;
AudioConnection      patchCord1(I2SIN, 0, INL, 0);
AudioConnection      patchCord2(I2SIN, 1, INR, 0);
AudioConnection      patchCord3(OUTL, 0, I2SOUT, 0);
AudioConnection      patchCord4(OUTR, 0, I2SOUT, 1);


// Effect Classes ////////////////////////////////////////////////////////////////

#ifdef FX0A
Looper looperA(blockSize, 441000, false);
#endif
#ifdef FX1A
Looper blooperA(blockSize, 22000, true);
#endif


// Setup ////////////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);
  
  pinMode(RSWA0, INPUT);
  pinMode(RSWA1, INPUT);
  pinMode(RSWA2, INPUT);
  pinMode(RSWB0, INPUT);
  pinMode(RSWB1, INPUT);
  pinMode(RSWB2, INPUT);
  pinMode(LAR, OUTPUT);
  pinMode(LAB, OUTPUT);
  pinMode(LBR, OUTPUT);
  pinMode(LBB, OUTPUT);

  setLedA(0); // Led rings are common anode thus inverting, would turn on if left low
  setLedB(0);

  fsa.attachClick(fsaClick); // Onebutton objects don't need normal pin initialization
  fsa.attachPressStart(fsaPressStart);
  fsa.attachLongPressStop(fsaPressStop);
  fsb.attachClick(fsbClick);
  fsb.attachPressStart(fsbPressStart);
  fsb.attachLongPressStop(fsbPressStop);

  fsa.setClickTicks(50); // These might require more tweaking.
  fsa.setPressTicks(200);
  fsb.setClickTicks(50);
  fsb.setPressTicks(200);

  wm8731.enable();
  wm8731.inputLevel(0.75); // Roughly gain matched with my preamps
  wm8731.inputSelect(AUDIO_INPUT_LINEIN);

  delay(1000); // WM8731 needs some time to boot
  
  AudioMemory(20);

  INL.begin();
  INR.begin();

  //MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
  //MIDI.begin(MIDI_CHANNEL_OMNI);
}


// Loop ////////////////////////////////////////////////////////////////

void loop()
{
  checkPresets();
  
  fsa.tick();
  fsb.tick();
  
  if(INL.available() && INR.available())
  {
    inL = INL.readBuffer();
    inR = INR.readBuffer();
    outL = OUTL.getBuffer();
    outR = OUTL.getBuffer();

    switch(presetA)
    {
      #ifdef FX0A
      case 0:
        looperA.processBlock(inL, inR, midL, midR);
        setLedA(looperA.checkLeds());
        break;
      #endif

      #ifdef FX1A
      case 1:
        blooperA.processBlock(inL, inR, midL, midR);
        setLedA(blooperA.checkLeds());
        break;
      #endif

      #ifdef FX2A
      case 2:
        //
        break;
      #endif

      #ifdef FX3A
      case 3:
        //      
        break;
      #endif

      #ifdef FX4A
      case 4:
        // 
        break;
      #endif

      #ifdef FX5A
      case 5:
        //
        break;
      #endif

      #ifdef FX6A
      case 6:
        //
        break;
      #endif

      #ifdef FX7A
      case 7:
        //
        break;
      #endif

      default:
        for(uint16_t i = 0; i < blockSize; i++) // Copy input buffer to output buffer
        {
          midL[i] = inL[i];
          midR[i] = inR[i];
        }
        break;
    } // switch  

    switch(presetB)
    {
      #ifdef FX0B
      case 0:
        looperB.processBlock(midL, midR, outL, outR);
        setLedB(looperB.checkLeds());
        break;
      #endif

      #ifdef FX1B
      case 1:
        blooperB.processBlock(midL, midR, outL, outR);
        setLedB(blooperB.checkLeds());
        break;
      #endif

      #ifdef FX2B
      case 2:
        //
        break;
      #endif

      #ifdef FX3B
      case 3:
        //      
        break;
      #endif

      #ifdef FX4B
      case 4:
        // 
        break;
      #endif

      #ifdef FX5B
      case 5:
        //
        break;
      #endif

      #ifdef FX6B
      case 6:
        //
        break;
      #endif

      #ifdef FX7B
      case 7:
        //
        break;
      #endif

      default:
        for(uint16_t i = 0; i < blockSize; i++) // Copy input buffer to output buffer
        {
          outL[i] = midL[i];
          outR[i] = midR[i];
        }
        break;
    } // switch  
     
    OUTL.playBuffer();
    OUTR.playBuffer();
  } // if
  INL.freeBuffer();
  INR.freeBuffer();
} // loop


// Rotary Switch Functions ////////////////////////////////////////////////////////////////

void checkPresets()
{
  presetA = digitalRead(RSWA0) + (digitalRead(RSWA1) << 1) + (digitalRead(RSWA2) << 2);
  presetB = digitalRead(RSWB0) + (digitalRead(RSWB1) << 1) + (digitalRead(RSWB2) << 2);
} // void checkPresets


// Footswitch A Callbacks ////////////////////////////////////////////////////////////////

void fsaClick()
{
  switch(presetA)
  {
    #ifdef FX0A
    case 0:
      looperA.fsClick();
      break;
    #endif

    #ifdef FX1A
    case 1:
      blooperA.fsClick();
      break;
    #endif
    
    default:
      break;
  }
} // void fsaClick

void fsaPressStart()
{
  switch(presetA)
  {
    #ifdef FX0A
    case 0:
      looperA.fsHold();
      break;
    #endif

    #ifdef FX1A
    case 1:
      blooperA.fsHold();
      break;
    #endif
    
    default:
      break;
  }
} // void fsaPressStart

void fsaPressStop()
{
  switch(presetA)
  {
    #ifdef FX0A
    case 0:
      looperA.fsStop();
      break;
    #endif

    #ifdef FX1A
    case 1:
      blooperA.fsStop();
      break;
    #endif
    
    default:
      break;
  }
} // void fsaPressStop


// Footswitch B Callbacks ////////////////////////////////////////////////////////////////

void fsbClick()
{
  switch(presetB)
  {
    #ifdef FX0B
    case 0:
      //
      break;
    #endif

    default:
      break;
  }
} // void fsbClick

void fsbPressStart()
{
  switch(presetB)
  { 
    #ifdef FX0B
    case 0:
      //
      break;
    #endif

    default:
      break;
  }    
} // void fsbPressStart

void fsbPressStop()
{
  switch(presetB)
  {
    #ifdef FX0B
    case 0:
      //
      break;
    #endif

    default:
      break;
  }    
} // void fsbPressStop


// Led Setter Functions ////////////////////////////////////////////////////////////////

void setLedA(uint8_t i)
{
  digitalWrite(LAB, !(i & 1));
  digitalWrite(LAR, !((i >> 1) & 1));
} // void setLedA

void setLedB(uint8_t i)
{
  digitalWrite(LBB, !(i & 1));
  digitalWrite(LBR, !((i >> 1) & 1));  
} // void setLedB
