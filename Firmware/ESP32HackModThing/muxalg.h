#ifndef __muxalg_include
#define __muxalg_include

#define NUM_MUX_POTS 16

void MuxSetup(void);
void MuxLoop(bool bVoltageReading=false);
void MuxLoopSingle (uint8_t pot, bool bVoltageReading=false);
uint16_t MuxAnalogRead (uint8_t pot);
uint32_t MuxAnalogReadMilliVolts (uint8_t pot);

#endif
