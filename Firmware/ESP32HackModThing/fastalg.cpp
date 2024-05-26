#include <Arduino.h>
#include "fastalg.h"

//--------------------------
//    Fast Analog Handling
//--------------------------
#define NUM_FAST_PINS (FAST_LAST_PIN+1)
int fastpins[NUM_FAST_PINS];
uint16_t fastPots[NUM_FAST_PINS];
uint32_t fastPotsMV[NUM_FAST_PINS];

void FastAlgSetup (int algpins[], int numpins) {
  for (int i=0; i<NUM_FAST_PINS; i++) {
    fastpins[i] = -1;
  }
  for (int i=0; i<numpins; i++) {
    if (algpins[i] < NUM_FAST_PINS) {
      fastpins[algpins[i]] = algpins[i];
    }
  }
}

void FastAlgLoop (bool bVoltageReading) {
  for (int i=0; i<NUM_FAST_PINS; i++) {
    if (fastpins[i] != -1) {
      FastAlgLoopSingle(fastpins[i], bVoltageReading);
    }
  }
}

void FastAlgLoopSingle (uint8_t pot, bool bVoltageReading) {
  if (pot < NUM_FAST_PINS) {
    if (bVoltageReading) {
      fastPotsMV[pot] = analogReadMilliVolts (pot);
    }
    else {
      fastPots[pot] = analogRead (pot);
    }
  }
}

// Can be used to grab a stored value
uint16_t FastAnalogRead (uint8_t pot) {
  if (pot < NUM_FAST_PINS) {
    return fastPots[pot];
  } else {
    return 0;
  }
}

uint32_t FastAnalogReadMilliVolts (uint8_t pot) {
  if (pot < NUM_FAST_PINS) {
    return fastPotsMV[pot];
  } else {
    return 0;
  }
}
