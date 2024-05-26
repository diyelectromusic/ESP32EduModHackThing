#include <Arduino.h>
#include "muxalg.h"

//--------------------------
//    MUX Analog Handling
//--------------------------
#define MUX_PINS 4
#define MUX_POTS NUM_MUX_POTS
int muxSPins[MUX_PINS] = {33,27,14,12}; // S0, S1, S2, S3
int muxAlgIn = 32;
uint16_t muxPots[MUX_POTS];
uint32_t muxPotsMV[MUX_POTS];

void MuxSetup () {
  for (int i=0; i<MUX_PINS; i++) {
    pinMode (muxSPins[i], OUTPUT);
    digitalWrite(muxSPins[i], LOW);
  }
}

void MuxLoop (bool bVoltageReading) {
  for (int i=0; i<MUX_POTS; i++) {
    MuxLoopSingle(i, bVoltageReading);
  }
}

void MuxLoopSingle (uint8_t pot, bool bVoltageReading) {
  if (pot<MUX_POTS) {
    for (int s=0; s<MUX_PINS; s++) {
      if (pot & (1<<s)) {
        digitalWrite(muxSPins[s],HIGH);
      } else {
        digitalWrite(muxSPins[s],LOW);
      }
    }
    if (bVoltageReading) {
      muxPotsMV[pot] = analogReadMilliVolts (muxAlgIn);
    }
    else {
      muxPots[pot] = analogRead (muxAlgIn);
    }
  }
}

// Can be used to grab a stored value
uint16_t MuxAnalogRead (uint8_t pot) {
  if (pot < MUX_POTS) {
    return muxPots[pot];
  } else {
    return 0;
  }
}

uint32_t MuxAnalogReadMilliVolts (uint8_t pot) {
  if (pot < MUX_POTS) {
    return muxPotsMV[pot];
  } else {
    return 0;
  }
}
