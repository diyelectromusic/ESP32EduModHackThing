#ifndef __env_include
#define __env_include

// ADSR state machine
enum adsr_t {
  adsrIdle=0,
  adsrTrigger,
  toAttack,  adsrAttack,
  toDelay,   adsrDelay,
  toSustain, adsrSustain,
  toRelease, adsrRelease,
  adsrReset
};

#define ATTACK_LEVEL 255     // Maximum level for attack
#define ATTACK_LEVEL_88      (256*ATTACK_LEVEL)      // In 8.8 format
#define ATTACK_LEVEL_MIN 0   // Usually zero unless want a minimum value
#define ATTACK_LEVEL_MIN_88  (256*ATTACK_LEVEL_MIN)  // In 8.8 format

class CEnv {
public:
  CEnv ();
  ~CEnv (void);

  // Inputs are potentiometer-scaled valus in range 0..4095
  void setADSR (int eg_a, int eg_d, int eg_s, int eg_r);

  uint8_t nextADSR (void);
  void triggerADSR (void);
  void gateADSR (bool gateOn);
  void dumpADSR (void);
  uint8_t getADSR (void);

private:
  // Envelope state/status
  //
  // NB: Levels in 8.8 fixed point values.
  //     Times in 0.1mS units.
  //
  // Amount to update the level for each state on each 'tick' is given by:
  //   Num Steps = Time (S) / Sample Rate (Hz) = Time (mS) / Sample Rate (kHz)
  //   Step = (End Level - Start Level) / Num Steps
  //
  // Use a signed 8.8 value for update per tick as can go up or down.
  //
  // NB: Must be careful env_l and steps don't wrap around, so
  //     use signed 32-bit values to make testing easier.
  //
  int32_t  env_l;
  int32_t  steps;
  uint16_t attack_ms;
  uint16_t attack_l;
  uint16_t delay_ms;
  int32_t  sustain_l;
  uint16_t release_ms;
  bool gate;  
  adsr_t state;
};

#endif
