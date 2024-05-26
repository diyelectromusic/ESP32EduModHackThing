#ifndef __osc_include
#define __osc_include

// For direct digital synthesis from a wavetable
// we have an accumulator to store the index into
// the table and an increment based on the sample
// rate and frequency.
//   Increment = Frequency * (Number of Samples in wavetable / Sample Rate)
//   Increment = Frequency * (256 / 32768)
//   Increment = Frequency / 128
//
// But using a 8.8 fixed-point accumulator and increment:
//   Increment = 256 * Frequency / 128
//   Increment = Frequency * 2
//
// Alternatively for a 16384 SAMPLE RATE:
//   Increment = 256 * Freq * (256 / 16384) = Freq * 4
//
#define FREQ2INC(f) (f*4)  // 16384 Hz
//#define FREQ2INC(f) (f*2)  // 32768 Hz

// Use a SAMPLE_RATE that is a multiple of the basic wavetable
#define NUM_SAMPLES    256
#define BASE_FREQ_MULT 64
#define SAMPLE_RATE (NUM_SAMPLES*BASE_FREQ_MULT)  // i.e. 256*64 = 16384 Hz
//#define BASE_FREQ_MULT 128
//#define SAMPLE_RATE (NUM_SAMPLES*BASE_FREQ_MULT)  // i.e. 256*128 = 32768 Hz

// 10 bit resoution for PWM will mean 78277 Hz PWM frequency
// 9 bit resoution for PWM will mean 156555 Hz PWM frequency
// 8 bit resoution for PWM will mean 313111 Hz PWM frequency
#define PWM_RESOLUTION 8
#define PWM_FREQUENCY  313111

#define OSC_SIN 0
#define OSC_SAW 1
#define OSC_TRI 2
#define OSC_SQU 3
#define OSC_WAVES 4

// Volume range for the oscillator.
//   Max must be (2^n - 1).
//   Scale is number of bits to ahift back for multiplying by a volume.
// Note: not much point being larger than 255/8-bits with a 8-bit wavetable.
//       (and if so, vol calculation would have to be larger than 16-bits too).
#define MAX_VOL   255
#define VOL_SCALE 8

// Frequency range for the LFO
#define LFO_MIN    1  // 0.1 Hz
#define LFO_MAX  500  // 0.1 Hz

// Set the frequency for 0V
#define FREQ_0V (65.406)  // C2

#define NO_MIDI_NOTE 255

class COsc {
public:
  COsc ();
  ~COsc (void);

  void Setup (int pwmpin, int wave, bool lfo=false);
  void QuadSetup (int sinpin, int sawpin, int tripin, int squpin);
  void ScanPWM (void);
  void SetFreq (uint16_t freq, uint16_t vol=MAX_VOL);
  void SetMVolts (uint32_t mvolts, float baseFreq=FREQ_0V, uint16_t vol=MAX_VOL);

  float MVolts2Freq (uint32_t mvolts, float baseFreq=FREQ_0V);
  float Midi2Freq (uint8_t note);

private:
  void setupWavetables (void);

private:
  int numwaves;
  int pwmpin[OSC_WAVES];
  uint8_t  *pW[OSC_WAVES];
  bool lfo;
  uint16_t acc;
  uint16_t inc;
  uint16_t amp;
  uint16_t lfocount;
  float basefreq;
};

#endif
