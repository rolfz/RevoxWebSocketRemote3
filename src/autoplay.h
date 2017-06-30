#ifndef __AUTOPLAY__
#define __AUTOPLAY__

#include <Arduino.h>
#include <EEPROM.h>

extern volatile int mainCnt; // Tape counter
extern volatile int task;
extern volatile int cntS[5];
extern volatile int cntE[5];
extern volatile int gotoPos;
extern volatile int startPos;
extern volatile int endPos;

void gotoPosition(uint8 * index);
void playMemory(uint8 * index);
void gotoZero(void);
void autoPlay(int state);

#endif
