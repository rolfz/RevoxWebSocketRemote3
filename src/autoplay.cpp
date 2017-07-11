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
volatile bool toPlay = false;
volatile bool repeat = false;
volatile uint32_t delayCnt=0;
// distance correction relative to speed 50/100/200/400
int const rewTab[]={25,40,60,80,100};
int const forTab[]={20,40,40,40,80};
bool const SPD_CORR =true;

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

     toPlay = true;
     task=GOTO;
  }

void autoPlay(int state){
  int corr;
  int dif;
//   if(state != END)Serial.println(stateTxt[state]);
   switch(state){

   case GOTO:
      if(mainCnt<gotoPos){
        // we end in forward
        dif=gotoPos-mainCnt;
        if(dif<50)corr=forTab[0];
        else if(dif<100)corr=forTab[1];
        else if(dif<200)corr=forTab[2];
        else if(dif<400)corr=forTab[3];
        else corr=forTab[4];

         updateValue("status","FORWARD");
         runForward();
         task=WAIT_FORWARD;
      }
      else if(mainCnt>gotoPos){
        // we end in reverse
        dif=mainCnt-gotoPos;
        if(dif<50)corr=forTab[0];
        else if(dif<100)corr=forTab[1];
        else if(dif<200)corr=forTab[2];
        else if(dif<400)corr=forTab[3];
        else corr=forTab[4];

         updateValue("status","REWIND");
         runRewind();
         task=WAIT_REWIND;
      }
      else {
        task=STOP;
      }
   break;

   case WAIT_REWIND:
        if(mainCnt<=gotoPos+corr){
           task=STOP;
        }
        break;

   case WAIT_FORWARD:
         if(mainCnt>=gotoPos-corr){
          task=STOP;
        }
        break;

   case STOP:
      updateValue("status","STOP");
      runStop();
      if(toPlay == true || repeat== true){ delayCnt=millis();
                          //  Serial.println(delayCnt);
                            task=WAIT_STOP;
                          }
              else task=END;
   break;
   case WAIT_STOP:
         if((millis()-delayCnt)>2000){ // tape stopped
           //Tape is stopped
           // we are here if we have to play or of we repeat
                if(repeat==true){
                      task=GOTO;
                      // if we repeat we will again play...
                      toPlay=true;
                    }
                else
                task=PLAY;
            }
   break;

      case PLAY:
           updateValue("status","PLAY");
           runPlay();
           task=WAIT_PLAY_END;
           toPlay=false;
  break;

  case WAIT_PLAY_END:
          if(mainCnt>=endPos){
             task=STOP;
          }
  break;

   case END:
   // do nothing
   break;
   default:
   break;
  }
} // end autoplay
