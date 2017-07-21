#ifndef __AUTOPLAY__
#define __AUTOPLAY__

#include <Arduino.h>
#include <EEPROM.h>

//extern volatile int16_t mainCnt; // Tape counter
extern volatile int16_t task;
extern volatile int16_t cntS[5];
extern volatile int16_t cntE[5];
extern volatile int16_t gotoPos;
extern volatile int16_t startPos;
extern volatile int16_t endPos;

enum autostate  {END,PLAY,WAIT_PLAY_END,GOTO,GOTOZERO,WAIT_FORWARD,WAIT_REWIND,WAIT_STOP,STOP};

void gotoPosition(uint8 * index);
void playMemory(uint8 * index);
void gotoZero(void);
void autoPlay(int state);
void stopZero(int16_t cnt);
int16_t speedCorr(int16_t dif,int16_t tab[] );

extern int16_t rewTab[5];
extern int16_t forTab[5];
extern volatile bool zeroStop;
extern volatile int16_t stopPos;

#endif
