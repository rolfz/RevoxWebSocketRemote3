/*
Autoplay code for movements on Revox B77 tape recorder

By Rolf Ziegler
June 2017

*/

#include "autoplay.h"
#include "MCP_code.h"

enum autostate  {PLAY,GOTO,FORWARD,REWIND,GOTOZERO,GOTOSTART,END,STOP};
volatile int task;

volatile int gotoPos=0;
volatile int startPos=0;
volatile int endPos=0;

void gotoPosition(uint8 * index){

      int loc=index[2]-'0';

      gotoPos=EEPROM.read(loc);
      gotoPos+=EEPROM.read(loc+1)<<8;
  // add code to goto here
      task=GOTO;

  }

void playMemory(uint8 loc){

      startPos=EEPROM.read(loc);
      startPos+=EEPROM.read(loc+1)<<8;

      endPos=EEPROM.read(loc+8);
      endPos+=EEPROM.read(loc+9)<<8;

      task=PLAY;
  }

void autoPlay(int state){

switch(state){
   case GOTO:
      if(gotoPos>mainCnt){
         runForward();
         state=FORWARD;
      }
      else{
        runRewind();
        state=REWIND;
      }
   break;
   case FORWARD:
     if(gotoPos<=mainCnt)
        state=STOP;
   break;
   case REWIND:
     if(gotoPos>mainCnt)
        state=STOP;
   break;
   case GOTOZERO:
     if(mainCnt>0){
       runRewind();
       state=STOP;
     }
     else { // mainCnt <= 0
       runForward();
       state=STOP;
     }
   break;
   case PLAY:

   break;
   case STOP:
     runStop();
     state=END;
   break;
   default:
   break;
  }
} // end autoplay
