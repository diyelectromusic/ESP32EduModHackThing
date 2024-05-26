#include <Arduino.h>
#include "env.h"

CEnv::CEnv (void) {
  attack_ms  = 500; // 50mS
  attack_l   = ATTACK_LEVEL_88;
  delay_ms   = 2000;  // 200mS
  sustain_l  = ATTACK_LEVEL_88 * 2 / 3;
  release_ms = 5000; // 500mS
  gate  = false;    
  state = adsrReset;
  env_l = ATTACK_LEVEL_MIN_88;
}

CEnv::~CEnv (void) {
}

void CEnv::setADSR (int eg_a, int eg_d, int eg_s, int eg_r) {
  attack_ms  = 1 + eg_a * 2; // 0..8191 in 0.1mS units. NB: Cannot be zero!
  delay_ms   = 1 + eg_d * 2;
  sustain_l  = map (eg_s, 0, 4095, ATTACK_LEVEL_MIN_88, ATTACK_LEVEL_88); // 0..255 in 8.8 form: (val * 256) >> 4
  release_ms = 1 + eg_r * 4; // 0.16380 in 0.1mS units - max 1.6 S release
}

uint8_t CEnv::nextADSR (void) {
  // Update the ADSR state machine
  //
  // NB: I could eliminate the transition stages by
  //     allowing fall-through of states, but I've
  //     opted to keep then as individual states for
  //     clarity.
  //
  //     The downside is that each transition adds an
  //     extra "tick" (0.1mS) to the timing of each
  //     stage of the ADSR.  But as these are probably
  //     coming from pot settings anyway, that is
  //     pretty much just "in the noise" anyway.
  //
  // Also note that removal of the Gate signal will
  // start the released regardless of what state it is in.
  //
  // Retriggering can also happen at any time and the envelope
  // will grow to the ATTACK_LEVEL from whatever level
  // is already set.
  //
  switch (state) {
    case adsrIdle:
      // Doing nothing until triggered
      break;

    case adsrTrigger:
    case toAttack:
      // Calculate the step value.
      // As each tick is 0.1mS and all timings
      // for stages are in 0.1mS units, the number
      // of steps per tick is always 1 and the
      // number of steps per stage is the same as the
      // stored time in 0.1mS.
      //
      // Step (8.8 format) = 256 * (End Level - Start Level) / Num Steps
      //
      // NB: Start from whatever the current level is to allow for
      //     retriggering whilst previous envelope is still running...
      //     Still using same steps/timings though.
      steps = (ATTACK_LEVEL_88 - ATTACK_LEVEL_MIN_88) / attack_ms;
      state = adsrAttack;
      break;

    case adsrAttack:
      if (gate) {
        if (env_l >= ATTACK_LEVEL_88) {
          // Reached top of the envelope.
          env_l = ATTACK_LEVEL_88;
          steps = 0;
          state = toDelay;
        }
      } else {
        // Gate Off - Jump to R stage;
        state = toRelease;
      }
      break;

    case toDelay:
      // Calculate next set of steps and move to D phase.
      steps = (sustain_l - env_l) / delay_ms;
      state = adsrDelay;
      break;

    case adsrDelay:
      if (gate) {
        if (env_l <= sustain_l) {
          // Reached sustain level so hold this level and move to S stage
          env_l = sustain_l;
          steps = 0;
          state = toSustain;
        }
      } else {
        // Gate Off - Jump to R stage;
        state = toRelease;
      }
      break;

    case toSustain:
       // Nothing extra to do, so move straight on
       state = adsrSustain;
    case adsrSustain:
      if (gate) {
        // Hold here whilst gate is ON
      } else {
        // Gate Off - Jump to R stage;
        state = toRelease;
      }
      break;
 
    case toRelease:
      steps = (ATTACK_LEVEL_MIN_88 - env_l) / release_ms;
      state = adsrRelease;
      break;
 
    case adsrRelease:
      // Test for zero
      if (env_l <= ATTACK_LEVEL_MIN_88) {
        // Envelope complete
        state = adsrReset;
      }
      break;

    case adsrReset:
      env_l = 0;
      steps = 0;
      state = adsrIdle;
      break;

    default:
      state = adsrReset;
  }

  // Update the envelope level according to the parameters set above.
  env_l = env_l + steps;
  if (env_l < ATTACK_LEVEL_MIN_88) {
    env_l = ATTACK_LEVEL_MIN_88;
  }
  if (env_l > ATTACK_LEVEL_88) {
    env_l = ATTACK_LEVEL_88;
  }

  return env_l >> 8;
}

uint8_t CEnv::getADSR (void) {
  // Return current level of the ADSR
  return env_l >> 8;
}

void CEnv::triggerADSR (void) {
  state = adsrTrigger;
}

void CEnv::gateADSR (bool gateOn) {
  if (gateOn) {
    gate = true;
  } else {
    gate = false;
  }
}

void CEnv::dumpADSR (void) {
  Serial.print(gate);
  Serial.print(":");
  Serial.print(state);
  Serial.print("\t");
  Serial.print(steps);
  Serial.print("\t[");
  Serial.print(env_l);
  Serial.print("]\t ADSR=");
  Serial.print(attack_ms);
  Serial.print(",\t");
  Serial.print(delay_ms);
  Serial.print(",\t");
  Serial.print(sustain_l);
  Serial.print(",\t");
  Serial.print(release_ms);
  Serial.print("\n");  
}
