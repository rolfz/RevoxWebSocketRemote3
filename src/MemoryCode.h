#ifndef __MEMORYCODE__
#define __MEMORYCODE__

#include <Arduino.h>
#include <EEPROM.h>


extern volatile int mainCnt; // Tape counter
extern volatile int cntS[5];
extern volatile int cntE[5];

void startEEPROM(void);
void storePosition(uint8 * index);
void storeMainCnt(void);
void restorePositions();
void gotoPosition(uint8 * index);
void playMemory(uint8 loc);
void dumpMemory(void);

#endif
