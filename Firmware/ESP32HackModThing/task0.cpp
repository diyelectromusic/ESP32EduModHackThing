#include <Arduino.h>
#include "task0.h"
#include "muxalg.h"
#include "fastalg.h"
#include "osc.h"

//#define DBG_TIMING
//#define ENABLE_TX

#ifdef DBG_TIMING
#warning Using Timing pins on 25 and 26
#define TIMING_PIN1 25
#define TIMING_PIN1_TOG
#define TIMING_PIN2 26
#define TIMING_PIN2_TOG
#endif

//--------------------------
//    Oscillator Setup
//--------------------------
#define OSC_VCO1 0
#define OSC_VCO2 1
#define OSC_LFO1 2
#define OSC_LFO2 3
#define NUM_OSC 4
COsc *pOsc[NUM_OSC];
int pwmpins[NUM_OSC][4] = {
  {17,18,5,19},
#ifdef ENABLE_TX
#warning PWM Disabled on GPIO 1 to enabled TX
  {21,22,0,23},
#else
  {21,22,1,23},
#endif
  {16,0,0,0},
  {13,0,0,0}
};
int oscwaves[NUM_OSC] = {
  OSC_WAVES,  // VCO 1 (quad)
  OSC_WAVES,  // VCO 2 (quad)
  OSC_TRI,    // LFO
  OSC_SAW
};

void oscSetup() {
  for (int i=0; i<NUM_OSC; i++) {
    pOsc[i] = new COsc();
    if (oscwaves[i] == OSC_WAVES) {
      // This is a quad oscillator
      pOsc[i]->QuadSetup (pwmpins[i][0], pwmpins[i][1], pwmpins[i][2], pwmpins[i][3]);
    } else {
      // A single oscillator - these are the LFOs
      pOsc[i]->Setup (pwmpins[i][0], oscwaves[i], true);      
    }
    pOsc[i]->SetFreq (440);
  }
}

//--------------------------
//    IO Configuration
//--------------------------
// Two values for each IO point:
//   - A real ALG connected input signal
//   - A MUX read pot
#define VCO1_CV    0
#define VCO1_AMP   1
#define VCO2_CV    2
#define NUM_ALGS   3
int mux_pins[NUM_ALGS] = {5,6,7};
uint32_t mux_val[NUM_ALGS];
int alg_pins[NUM_ALGS] = {15,4,2};
uint32_t alg_val[NUM_ALGS];
#define LFO_R_MUX  1 // MUX PIN
#define LFO_D_MUX  2 // MUX PIN
uint16_t alg_lfo_r;
uint16_t alg_lfo_d;
float midiFreq=FREQ_0V;

void Task0AlgIOLoop (void) {
  // Scan in all the IO that is configured
  
  // Read the CVs as voltages
  mux_val[VCO1_CV] = MuxAnalogReadMilliVolts(mux_pins[VCO1_CV]);
  alg_val[VCO1_CV] = FastAnalogReadMilliVolts(alg_pins[VCO1_CV]);
  
  mux_val[VCO2_CV] = MuxAnalogReadMilliVolts(mux_pins[VCO2_CV]);
  alg_val[VCO2_CV] = FastAnalogReadMilliVolts(alg_pins[VCO2_CV]);

  // Read the amplitude as RAW values
  mux_val[VCO1_AMP] = MuxAnalogRead(mux_pins[VCO1_AMP]);
  alg_val[VCO1_AMP] = FastAnalogRead(alg_pins[VCO1_AMP]);

  // Read the LFO controls as RAW values
  alg_lfo_r = MuxAnalogRead(LFO_R_MUX);
  alg_lfo_d = MuxAnalogRead(LFO_D_MUX);
}

//--------------------------
//    Specific VCO/LFO
//--------------------------
void SetVCO1Freq (void) {
  // Frequency is a simple addition.
  // This allows the use of both the CV and knob at the same time.
  uint32_t mV = mux_val[VCO1_CV] + alg_val[VCO1_CV];

  // Amplitude is the maximum of the two values scaled to MAX_VOL.
  uint16_t amp = (mux_val[VCO1_AMP] > alg_val[VCO1_AMP]) ? mux_val[VCO1_AMP] : alg_val[VCO1_AMP];
  amp = map (amp, 0, 4095, 0, MAX_VOL);

  pOsc[OSC_VCO1]->SetMVolts (mV, midiFreq, amp);
}

void SetVCO2Freq (void) {
  // Just frequency to set
  uint32_t mV = mux_val[VCO2_CV] + alg_val[VCO2_CV];

  pOsc[OSC_VCO2]->SetMVolts (mV, midiFreq);
}

void SetLFOFreq (void) {
  // LFO rate and depth just read directly
  uint16_t lforate = map (alg_lfo_r, 0, 4095, LFO_MIN, LFO_MAX);
  uint16_t lfodepth = map (alg_lfo_d, 0, 4095, 0, MAX_VOL);

  pOsc[OSC_LFO1]->SetFreq (lforate, lfodepth);
  pOsc[OSC_LFO2]->SetFreq (lforate, lfodepth);
}

//--------------------------
//    MIDI Freq Handling
//--------------------------
void setMidiFrequency(uint8_t pitch) {
  // Note: there is just a single global MIDI frequency that affects both VCOs
  midiFreq = pOsc[0]->Midi2Freq (pitch);
}

void clearMidiFrequency(void) {
  midiFreq = FREQ_0V;
}

//--------------------------
//    ESP 32 Timer ISR
//--------------------------

// Timer configuration
// NB: CPU Freq = 80MHz so dividers used to get other
//     frequencies for syncing to other operations.
//
// For a timer frequency of 1000,000Hz i.e. divider of 80,
// the Timer alarms would be configured in units of 1uS.
//
// So need to convert required sample rate into uS.
//    Period = 1 / Sample Rate = 1 / 32768 = 30.5uS
//
// However, if up the timer frequency to 10MHz then
// alarms will be set in 0.1uS.
//
// Note: Although the timer is running at 32768Hz,
//       the PWM outputs are services in two blocks
//       on every other scan, so the actual effective
//       sample rate supported is 16384Hz
//
#define TIMER_FREQ  10000000  // 1MHz * 10 (0.1uS)
#define TIMER_RATE  305       // 30.5 uS * 10

// Use the ESP32 timer routines
hw_timer_t *Task0Timer = NULL;

int toggle0;
void ARDUINO_ISR_ATTR Task0TimerIsr (void) {
#ifdef TIMING_PIN1
#ifdef TIMING_PIN1_TOG
  if (toggle0) {
    digitalWrite(TIMING_PIN1, HIGH);
  } else {
    digitalWrite(TIMING_PIN1, LOW);
  }
#else
  digitalWrite(TIMING_PIN1, HIGH);
#endif
#endif

  toggle0 = !toggle0;
  if (toggle0) {
    pOsc[OSC_VCO1]->ScanPWM();
    pOsc[OSC_LFO1]->ScanPWM();
  } else {
    pOsc[OSC_VCO2]->ScanPWM();
    pOsc[OSC_LFO2]->ScanPWM();
  }

#ifdef TIMING_PIN1
#ifndef TIMING_PIN1_TOG
  digitalWrite(TIMING_PIN1, LOW);
#endif
#endif
}

void Task0TimerSetup () {
  Task0Timer = timerBegin(TIMER_FREQ);
  timerAttachInterrupt(Task0Timer, &Task0TimerIsr);
  timerAlarm(Task0Timer, TIMER_RATE, true, 0);
}

//--------------------------
//    Setup and Loop
//--------------------------

void Task0Setup(void)
{
#ifdef TIMING_PIN1
  pinMode (TIMING_PIN1, OUTPUT);
#endif
#ifdef TIMING_PIN2
  pinMode (TIMING_PIN2, OUTPUT);
#endif

  oscSetup();

  Task0TimerSetup();
}

int timingtog0;
void Task0Loop(void)
{
#ifdef TIMING_PIN2
  timingtog0 = !timingtog0;
#ifdef TIMING_PIN2_TOG
  if (timingtog0) {
    digitalWrite(TIMING_PIN2, HIGH);
  } else {
    digitalWrite(TIMING_PIN2, LOW);
  }
#else
    digitalWrite(TIMING_PIN2, HIGH);
#endif
#endif

  Task0AlgIOLoop();
  SetVCO1Freq();
  SetVCO2Freq();
  SetLFOFreq();

#ifdef TIMING_PIN2
#ifndef TIMING_PIN2_TOG
  digitalWrite(TIMING_PIN2, LOW);
#endif
#endif
}
