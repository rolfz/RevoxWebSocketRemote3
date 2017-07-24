#ifndef __MEMORYCODE__
#define __MEMORYCODE__

#include <Arduino.h>
#include <EEPROM.h>
#include "EncoderCode.h"

extern volatile int16_t cntS[5];
extern volatile int16_t cntE[5];

void startEEPROM(void);
int16_t storePosition(uint8_t * index, int16_t cnt);
void storeMainCnt(void);
void storeOffset(const char * index, int16_t value);
void restoreOffset(void);
void restoreCounters();
void dumpMemory(void);

#endif
