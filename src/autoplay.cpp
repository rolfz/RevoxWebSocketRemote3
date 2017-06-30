/*
Autoplay code for movements on Revox B77 tape recorder

By Rolf Ziegler
June 2017

*/

#include "autoplay.h"
#include "MCP_code.h"
#include "WEB_Code.h"

enum autostate  {END,PLAY,WAIT_PLAY_END,GOTO,GOTOZERO,WAIT_FORWARD,WAIT_REWIND,WAIT_STOP,STOP};

char  const* stateTxt[]= {"End","Play","Playing","Goto","GoZero","WaitFOR","WaitREW","WaitStop","Stop"};

volatile int task;

volatile int gotoPos=0;
volatile int startPos=0;
volatile int endPos=0;
volatile bool fromplay = false;

void gotoPosition(uint8 * index){

      int loc=index[2]-'0';
      loc*=2;
      if(index[1]=='G'){
                gotoPos=EEPROM.read(loc);
                gotoPos+=EEPROM.read(loc+1)<<8;
                }
                else
                {
                gotoPos=EEPROM.read(loc+8);
                gotoPos+=EEPROM.read(loc+9)<<8;
                }
//      Serial.print("gotoPos: ");
//      Serial.println(gotoPos);
  // add code to goto here
      task=GOTO;
  }

  void gotoZero(void){

        gotoPos=0;
    // add code to goto here
        task=GOTO;

    }

void playMemory(uint8 * index){

      int loc=index[2]-'0';
      loc*=2;

      gotoPos=EEPROM.read(loc);
      gotoPos+=EEPROM.read(loc+1)<<8;

      endPos=EEPROM.read(loc+8);
      endPos+=EEPROM.read(loc+9)<<8;

/*      Serial.print("startPos: ");
      Serial.println(startPos);
      Serial.print("endPos: ");
      Serial.println(endPos);
  */
     fromplay = true;
     task=GOTO;
  }

void autoPlay(int state){

//   if(state != END)Serial.println(stateTxt[state]);

   switch(state){

   case GOTO:
      if(mainCnt<gotoPos){
         runForward();
         updateValue("status","FORWARD");
         task=WAIT_FORWARD;
      }
      else if(mainCnt>gotoPos){
         updateValue("status","REWIND");
         runRewind();
         task=WAIT_REWIND;
      }
      else {
        task=STOP;
      }
   break;

   case PLAY:
        updateValue("status","PLAY");
        runPlay();
        task=WAIT_PLAY_END;
        fromplay=false;
   break;

   case WAIT_PLAY_END:
       if(mainCnt>=endPos){
          task=STOP;
       }
       break;

   case WAIT_REWIND:
        if(mainCnt<=gotoPos){
           task=STOP;
        }
        break;

   case WAIT_FORWARD:
         if(mainCnt>=gotoPos){

          task=STOP;
        }
        break;
  case WAIT_STOP:
        if(tapeMove()==0){
            task=PLAY;
        }
        break;
   case STOP:
      updateValue("status","STOP");
      runStop();
      if(fromplay == true) task=WAIT_STOP;
                  else task=END;
   break;
   case END:
   // do nothing
   break;
   default:
   break;
  }
} // end autoplay
