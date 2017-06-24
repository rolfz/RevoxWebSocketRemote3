#ifndef __ENCODERCODE__
#define __ENCODERCODE__

#include <Arduino.h>
#include "MCP_Code.h"

// prototypes
void doEncoderB(void);
void doEncoderA(void);
void startEncoder(void);

extern volatile int mainCnt; // Tape counter

#endif
