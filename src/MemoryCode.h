#ifndef __MEMORYCODE__
#define __MEMORYCODE__

#include <Arduino.h>
#include <EEPROM.h>
#include "EncoderCode.h"

extern volatile int16_t cntS[5];
extern volatile int16_t cntE[5];

void startEEPROM(void);
int storePosition(uint8 * index);
void storeMainCnt(void);
void storeOffset(const char * index, int16_t value);
void restoreOffset(void);
void restoreCounters();
void dumpMemory(void);

#endif
