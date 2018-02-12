#ifndef __ENCODERCODE__
#define __ENCODERCODE__

#include <Arduino.h>
#include "MCP_Code.h"
#include "Pin_Def.h"
// prototypes
void doEncoderB(void);
void doEncoderA(void);
void startEncoder(void);

extern volatile int16_t mainCnt; // Tape counter

#endif
