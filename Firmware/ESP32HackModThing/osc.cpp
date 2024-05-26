#include <Arduino.h>
#include "osc.h"
#include "midi2freq.h"

#define WT_ZERO 128  // The mid-range "zero" value
static uint8_t sindata[NUM_SAMPLES] = {
  128, 131, 134, 137, 140, 144, 147, 150, 153, 156, 159, 162, 165, 168, 171, 174,
  177, 179, 182, 185, 188, 191, 193, 196, 199, 201, 204, 206, 209, 211, 213, 216,
  218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 239, 240, 241, 243, 244,
  245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255,
  255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246,
  245, 244, 243, 241, 240, 239, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220,
  218, 216, 213, 211, 209, 206, 204, 201, 199, 196, 193, 191, 188, 185, 182, 179,
  177, 174, 171, 168, 165, 162, 159, 156, 153, 150, 147, 144, 140, 137, 134, 131,
  128, 125, 122, 119, 116, 112, 109, 106, 103, 100,  97,  94,  91,  88,  85,  82,
  79,  77,  74,  71,  68,  65,  63,  60,  57,  55,  52,  50,  47,  45,  43,  40,
  38,  36,  34,  32,  30,  28,  26,  24,  22,  21,  19,  17,  16,  15,  13,  12,
  11,  10,   8,   7,   6,   6,   5,   4,   3,   3,   2,   2,   2,   1,   1,   1,
  1,   1,   1,   1,   2,   2,   2,   3,   3,   4,   5,   6,   6,   7,   8,  10,
  11,  12,  13,  15,  16,  17,  19,  21,  22,  24,  26,  28,  30,  32,  34,  36,
  38,  40,  43,  45,  47,  50,  52,  55,  57,  60,  63,  65,  68,  71,  74,  77,
  79,  82,  85,  88,  91,  94,  97, 100, 103, 106, 109, 112, 116, 119, 122, 125
};

// Following wavetables are calculated...
static uint8_t sawdata[NUM_SAMPLES];
static uint8_t tridata[NUM_SAMPLES];
static uint8_t squdata[NUM_SAMPLES];

COsc::COsc (void)
: numwaves(0), pwmpin(0), pW(nullptr), lfo(false), acc(0), inc(0), amp(MAX_VOL), lfocount(0), basefreq(FREQ_0V)
{
}

COsc::~COsc (void) { 
}

void COsc::Setup (int pin, int wt, bool islfo) {
  setupWavetables();
  numwaves = 1;
  pwmpin[0] = pin;
  switch (wt) {
    case OSC_SAW:
      pW[0] = sawdata;
      break;
    case OSC_SQU:
      pW[0] = squdata;
      break;
    case OSC_TRI:
      pW[0] = tridata;
      break;
    default:
      pW[0] = sindata;
      break;
  }

  lfo = islfo;

  ledcAttach(pwmpin[0], PWM_FREQUENCY, PWM_RESOLUTION);
}

void COsc::QuadSetup (int sinpin, int sawpin, int tripin, int squpin) {
  setupWavetables();
  numwaves = OSC_WAVES;
  pwmpin[OSC_SIN] = sinpin;
  pwmpin[OSC_SAW] = sawpin;
  pwmpin[OSC_TRI] = tripin;
  pwmpin[OSC_SQU] = squpin;
  pW[OSC_SIN] = sindata;
  pW[OSC_SAW] = sawdata;
  pW[OSC_TRI] = tridata;
  pW[OSC_SQU] = squdata;

  for (int i=0; i<OSC_WAVES; i++) {
    if (pwmpin[i] != 0) {
      ledcAttach(pwmpin[i], PWM_FREQUENCY, PWM_RESOLUTION);
    }
  }
}

void COsc::ScanPWM (void) {
  if (inc == 0) {
    // output mid range for zero
    for (int i=0; i<numwaves; i++) {
      ledcWrite (pwmpin[i], WT_ZERO);  
    }
  } else {
    if (lfo) {
      // LFO updates 10 times more slowly
      lfocount++;
      if (lfocount > 10) {
        lfocount = 0;
        acc += inc;
      }
    }
    else {
      acc += inc;
    }
    for (int i=0; i<numwaves; i++) {
      ledcWrite (pwmpin[i], (amp * pW[i][acc >> 8]) >> VOL_SCALE);
    }
  }
}

void COsc::SetFreq (uint16_t freq, uint16_t vol) {
  if (freq > 0) {
    inc = FREQ2INC(freq);
    amp = vol;
  }
}

void COsc::SetMVolts (uint32_t mvolts, float baseFreq, uint16_t vol) {
  float freq = MVolts2Freq(mvolts, baseFreq);
  SetFreq((uint16_t)freq, vol);
}

float COsc::MVolts2Freq (uint32_t mvolts, float baseFreq) {
  // Set frequency according to Volt/Oct scaling
  //   Freq = baseFreq * 2 ^ (voltage)
  //
  return baseFreq * (powf (2.0, ((float)mvolts)/1000.0));
}

float COsc::Midi2Freq (uint8_t note) {
  // Assuming MIDI note 69 is A 440Hz
  //return 440.0 * powf (2.0, ((float)note-69.0)/12.0);
  if (note < 128) {
    // Return value from the look-up table
    return midi2freq[note];
  } else {
    return 0.0;
  }
}

void COsc::setupWavetables (void) {
  if ((sawdata[42] != 42) && (tridata[42] != (42*2))) {
    // Need to initialise the wavetables on first run through
    for (int i=0; i<NUM_SAMPLES; i++) {
      sawdata[i] = NUM_SAMPLES-i;
      if (i<NUM_SAMPLES/2) {
        tridata[i] = i*2;
        squdata[i] = 255;
      } else {
        tridata[i] = 255-(i-128)*2;
        squdata[i] = 0;
      }
    }
  }
}
