#ifndef __fastalg_include
#define __fastalg_include

// Highest numbered analog pin covered
#define FAST_LAST_PIN 15

void FastAlgSetup (int algpins[], int numpins);
void FastAlgLoop (bool bVoltageReading=false);
void FastAlgLoopSingle (uint8_t pot, bool bVoltageReading=false);
uint16_t FastAnalogRead (uint8_t pot);
uint32_t FastAnalogReadMilliVolts (uint8_t pot);

#endif
