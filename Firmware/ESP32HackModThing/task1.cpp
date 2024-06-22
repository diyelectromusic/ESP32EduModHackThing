#include <Arduino.h>
#include <MIDI.h>
#include "task1.h"
#include "muxalg.h"
#include "fastalg.h"
#include "env.h"
#include "task0.h" // for MIDI frequency handling

//#define DBG_TIMING
//#define DBG_DUMP_ADSR

#ifdef DBG_TIMING
#define DISABLE_DAC
#warning Using Timing pins on 25 and 26
#define TIMING_PIN1 25
//#define TIMING_PIN1_TOG
#define TIMING_PIN2 26
#define TIMING_PIN2_TOG
#endif

//--------------------------
//    MIDI Setup
//--------------------------
MIDI_CREATE_DEFAULT_INSTANCE();
#define MIDI_CHANNEL MIDI_CHANNEL_OMNI

//--------------------------
//    IO Configuration
//--------------------------
#define NUM_DAC_PINS 2  // Also the number of EGs possible
int dac_pins[NUM_DAC_PINS] = {25, 26};
#define NUM_ADC_PINS 4  // Per DAC
// NB: Analog IO is via the MUX
int adc_pins[NUM_DAC_PINS][NUM_ADC_PINS] = {
   3, 4, 9, 8,  // EG 1/ADSR
  15,14,13,12   // EG 2/ADSR
};
uint16_t adcval[NUM_DAC_PINS][NUM_ADC_PINS];

void dacOutput (uint8_t pin, uint8_t value) {
#ifndef DISABLE_DAC
  dacWrite (pin, value);
#endif  
}

#define FAST_ALG_PINS 3
int fastalgmv[FAST_ALG_PINS]   = {true, false, true};  // ReadMilliVolts or not
int fastalgpins[FAST_ALG_PINS] = {15,4,2};

//--------------------------
//    Env Gen Setup
//--------------------------
#define NUM_ENV NUM_DAC_PINS
CEnv *pEnv[NUM_ENV];

int trig_pins[NUM_ENV] = {35,39};
int gate_pins[NUM_ENV] = {34,36};
int lasttrig[NUM_ENV];
int lastgate[NUM_ENV];
bool adsrgate[NUM_ENV];
bool midigate;

void envSetup() {
  for (int i=0; i<NUM_ENV; i++) {
    pEnv[i] = new CEnv();
    lasttrig[i] = HIGH;
    lastgate[i] = HIGH;
    adsrgate[i] = false;
  }
  midigate = false;
}

void envTrigger (void) {
  for (int i=0; i<NUM_ENV; i++) {
    pEnv[i]->triggerADSR();
  }
}

void envGate (void) {
  for (int i=0; i<NUM_ENV; i++) {
    if (midigate || adsrgate[i]) {
      // If either gate is on, then gate is on
      pEnv[i]->gateADSR (true);
    }
    else if (!midigate && !adsrgate[i]) {
      // If both gates are off, then gate is off
      pEnv[i]->gateADSR (false);
    }
  }
}

void envScan (void) {
  // Still not sure of the behaviour on MIDI NoteOff for pitch...
  // Should it reset the frequency directly on receiving NoteOff?
  // But then any envelope Release stage resets to the base pitch too...
  // Or maybe just leave it at the last pitch?
  return;
  
  // Check status of both envelopes and MIDI gate
  // and if both have stopped then reset the midi frequency.
  uint8_t res=0;
  if (!midigate) {
    for (int i=0; i<NUM_ENV; i++) {
      if (pEnv[i]->getADSR () != 0) {
        res = res | (1<<i);
      }
    }
    if (res == 0) {
      // All envelopes complete
      clearMidiFrequency ();
    }
  }
}

//--------------------------
//    IO Loops
//--------------------------
void Task1FastAlgIOLoop (void) {
  for (int i=0; i<FAST_ALG_PINS; i++) {
    FastAlgLoopSingle(fastalgpins[i], fastalgmv[i]);
  }
}

void Task1AlgIOLoop (void) {
  for (unsigned i=0; i<NUM_DAC_PINS; i++) {
    // cycle through each defined potentiometer
    bool setAdsr = false;
    for (unsigned p=0; p<NUM_ADC_PINS; p++) {
        uint16_t algval = MuxAnalogRead(adc_pins[i][p]);
        if (algval != adcval[i][p]) {
          setAdsr = true;
        }
        adcval[i][p] = algval;
    }

    if (setAdsr) {
      pEnv[i]->setADSR(adcval[i][0], adcval[i][1], adcval[i][2], adcval[i][3]);
    }
  }  
}

void Task1DigIOLoop (void) {
  // Check gate/triggers
  for (unsigned i=0; i<NUM_DAC_PINS; i++) {
    // NB: Triggers are active LOW
    int trigval = digitalRead(trig_pins[i]);
    if ((trigval == LOW) && (lasttrig[i] == HIGH)) {
      pEnv[i]->triggerADSR();
    }
    else {
      // No change
    }
    lasttrig[i] = trigval;

    // NB: Gates are active LOW
    int gateval = digitalRead(gate_pins[i]);
    if ((gateval == LOW) && (lastgate[i] == HIGH)) {
      // Gate is ON
      adsrgate[i] = true;
      envGate();
    }
    else if ((gateval == HIGH) && (lastgate[i] == LOW)) {
      // Gate is OFF
      adsrgate[i] = false;
      envGate();
    }
    else {
      // No change
    }
    lastgate[i] = gateval;
#ifdef DBG_DUMP_ADSR
    if (i==0) {
      Serial.print(trigval);
      Serial.print("\t");
      Serial.print(lasttrig[i]);
      Serial.print("\t");
      Serial.print(gateval);
      Serial.print("\t");
      Serial.print(lastgate[i]);
      Serial.print("\t");
      pEnv[i]->dumpADSR();
    }
#endif
  }
}

//--------------------------
//    ESP 32 Timer ISR
//--------------------------

// Timer configuration
// NB: CPU Freq = 80MHz so dividers used to get other
//     frequencies for syncing to other operations.
//
// For a timer frequency of 100,000Hz i.e. divider of 800,
// the Timer alarms would be configured in units of 10uS.
//
// I'm after a sample rate of 10kHz, which requires a 100uS
// period, so I'm going to trigger an alarm every 10 'ticks'.
//
#define SAMPLE_RATE_KHZ 10  // 10kHz (0.1mS)
#define TIMER_FREQ  100000  // 100kHz (0.01mS)
#define TIMER_RATE  10      // 0.1mS

// Use the ESP32 timer routines
hw_timer_t *Task1Timer = NULL;

int toggle1;
uint8_t sample[NUM_DAC_PINS];
void ARDUINO_ISR_ATTR Task1TimerIsr (void) {
#ifdef TIMING_PIN1
#ifdef TIMING_PIN1_TOG
  if (toggle1) {
    digitalWrite(TIMING_PIN1, HIGH);
  } else {
    digitalWrite(TIMING_PIN1, LOW);
  }
#else
  digitalWrite(TIMING_PIN1, HIGH);
#endif
#endif
  toggle1 = !toggle1;

  // Write out last value first to remove
  // any jitter from the timing of the ISR.
  for (unsigned i=0; i<NUM_DAC_PINS; i++) {
    dacOutput(dac_pins[i], sample[i]);
  }
  
  // Then calculate the new samples for next time
  for (unsigned i=0; i<NUM_DAC_PINS; i++) {
    sample[i] = pEnv[i]->nextADSR ();
  }
  
#ifdef TIMING_PIN1
#ifndef TIMING_PIN1_TOG
  digitalWrite(TIMING_PIN1, LOW);
#endif
#endif
}

void Task1TimerSetup () {
  Task1Timer = timerBegin(TIMER_FREQ);
  timerAttachInterrupt(Task1Timer, &Task1TimerIsr);
  timerAlarm(Task1Timer, TIMER_RATE, true, 0);
}

//--------------------------
//    MIDI Handling
//--------------------------
byte playing;
void MidiNoteOff(byte channel, byte pitch, byte velocity) {
  // If we clear the frequency now, then any release stage of the env
  // will instantly switch to the original reference frequency...
  //clearMidiFrequency ();
  if (playing == pitch) {
    // Only turn off the gate if received a matching note off to the
    // currently playing note, otherwise overlapping On/Off events
    // gets very messy.
    midigate = false;
    envGate();
    playing = -1;
  }
}

void MidiNoteOn(byte channel, byte pitch, byte velocity) {
  if (velocity == 0) {
    MidiNoteOff (channel, pitch, velocity);
    return;
  }
  setMidiFrequency (pitch);
  playing = pitch;
  midigate = true;
  envGate();
  envTrigger();
}

void MidiSetup (void) {
  MIDI.begin(MIDI_CHANNEL);
  MIDI.turnThruOff (); // Don't want to spam TX pin as using that for PWM
  MIDI.setHandleNoteOn(MidiNoteOn);
  MIDI.setHandleNoteOff(MidiNoteOff);
}

void Task1MIDILoop() {
  MIDI.read();
}

//--------------------------
//    Setup and Loop
//--------------------------

void Task1Setup(void)
{
#ifdef TIMING_PIN1
  pinMode (TIMING_PIN1, OUTPUT);
#endif
#ifdef TIMING_PIN2
  pinMode (TIMING_PIN2, OUTPUT);
#endif

  for (int i=0; i<NUM_DAC_PINS; i++) {
    pinMode (trig_pins[i], INPUT);
    lasttrig[i] = HIGH;
    pinMode (gate_pins[i], INPUT);
    lastgate[i] = HIGH;

    // Start with output at zero
    sample[i] = ATTACK_LEVEL_MIN;
    dacOutput(dac_pins[i], ATTACK_LEVEL_MIN);
    
    // Force a read of the ADCs on power up
    for (int p=0; p<NUM_ADC_PINS; p++) {
      adcval[i][p] = 32768;
    }
  }

  FastAlgSetup (fastalgpins, FAST_ALG_PINS);
  envSetup();
  Task1TimerSetup();
  MuxSetup();
  MidiSetup();
}

#define S_MUX (NUM_MUX_POTS-1)
#define S_DIGIO  (S_MUX+1)
#define S_ALGIO  (S_DIGIO+1)
#define S_LAST   (S_ALGIO+1)
// Not scanning following in the sequence atm - scanning every loop instead
#define S_FASTIO (S_ALGIO+1)
#define S_MIDI   (S_FASTIO+1)
int taskState;
int timingtog1;
void Task1Loop(void)
{
#ifdef TIMING_PIN2
  timingtog1 = !timingtog1;
#ifdef TIMING_PIN2_TOG
  if (timingtog1) {
    digitalWrite(TIMING_PIN2, HIGH);
  } else {
    digitalWrite(TIMING_PIN2, LOW);
  }
#else
    digitalWrite(TIMING_PIN2, HIGH);
#endif
#endif

  // Fast/Analog inputs and MIDI scanned every time
  Task1FastAlgIOLoop();
  Task1MIDILoop();

  if (taskState <= S_MUX) {
    // Read each MUX pot in turn
    switch (taskState) {
      case 5: // VCO 1 CV
      case 7: // VCO 2 CV
        // These are read in millivolts...
        MuxLoopSingle(taskState, true);
        break;
      default:
        // These are read as 12-bit raw 0..4095
        MuxLoopSingle(taskState, false);
        break;
    }
  }
  else if (taskState == S_FASTIO) {
    Task1FastAlgIOLoop();
  }
  else if (taskState == S_DIGIO) {
    Task1DigIOLoop();
  }
  else if (taskState == S_ALGIO) {
    Task1AlgIOLoop();
  }
  else if (taskState == S_MIDI) {
    Task1MIDILoop();
  }
  else {
    // Reset state as something weird going on
    taskState = S_LAST;
  }
  taskState++;
  if (taskState >= S_LAST) {
    taskState = 0;
  }

  envScan();

#ifdef TIMING_PIN2
#ifndef TIMING_PIN2_TOG
  digitalWrite(TIMING_PIN2, LOW);
#endif
#endif
}
