/*
Autoplay code for movements on Revox B77 tape recorder

By Rolf Ziegler
June 2017

*/

#include "autoplay.h"
#include "MCP_code.h"

void gotoPosition(uint8 * index){

  int loc=index[2]-'0';

  int gotoPos=EEPROM.read(loc);
      gotoPos+=EEPROM.read(loc+1)<<8;
  // add code to goto here
      if(gotoPos>mainCnt)
          //rewind
          {
          runForward();
        // we wait until the position is reached
         while(gotoPos>mainCnt);
          }
          else
          //foreward
          {
          runRewind();
          while(gotoPos>mainCnt);
          }
          runStop();
  }

void playMemory(uint8 loc){

  int startPos=EEPROM.read(loc);
      startPos+=EEPROM.read(loc+1)<<8;

  int endPos=EEPROM.read(loc+8);
      endPos+=EEPROM.read(loc+9)<<8;


  }
