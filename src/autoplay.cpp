/*
Autoplay code for movements on Revox B77 tape recorder
By Rolf Ziegler
June-Juillet 2017

*/
#include "autoplay.h"
#include "MCP_code.h"
#include "WEB_Code.h"
#include "EncoderCode.h"

char  const* stateTxt[]= {"End","Play","Playing","Goto","GoZero","WaitFOR","WaitREW","WaitStop","Stop"};

volatile int16_t task;
//test
volatile int16_t gotoPos=0;
volatile int16_t startPos=0;
volatile int16_t endPos=0;
volatile bool toPlay = false;
volatile bool repeat = false;
volatile uint32_t delayCnt=0;
volatile bool zeroStop = true;
volatile int16_t stopPos=0;

// distance correction relative to speed 50/100/200/400
int16_t rewTab[6]={-25,-40,-60,-80,-100,-120};
int16_t forTab[6]={20,40,40,40,80,100};

bool const SPD_CORR =true;

int16_t speedCorr(int16_t dif,int16_t tab[] ){

    int16_t corr;
    if(dif<50)corr=tab[0];
    else if(dif<100)corr=tab[1];
    else if(dif<200)corr=tab[2];
    else if(dif<400)corr=tab[3];
    else if(dif<800)corr=tab[4];
    else corr=tab[5];
    return corr;
}

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
  static int corr;
  static int dif;
//   if(state != END)Serial.println(stateTxt[state]);
   switch(state){

   case GOTO:
      if(mainCnt<gotoPos){
        // we end in forward
        //dif=gotoPos-mainCnt;
        corr=speedCorr(gotoPos-mainCnt,forTab );
        /*
        if(dif<50)corr=forTab[0];
        else if(dif<100)corr=forTab[1];
        else if(dif<200)corr=forTab[2];
        else if(dif<400)corr=forTab[3];
        else corr=forTab[4];
        */
         updateValue("status","FORWARD");
         runForward();
         task=WAIT_FORWARD;
      }
      else if(mainCnt>gotoPos){
        // we end in reverse
        /*dif=mainCnt-gotoPos;
        if(dif<50)corr=rewTab[0];
        else if(dif<100)corr=rewTab[1];
        else if(dif<200)corr=rewTab[2];
        else if(dif<400)corr=rewTab[3];
        else corr=rewTab[4];
        */
         corr=speedCorr(mainCnt-gotoPos,rewTab );
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
         if(mainCnt>=gotoPos+corr){
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

// code to stop the recorder at zero when rewinding
// needs maincounter and stop correction to stop at 0
  void stopZero(int16_t cnt){
    if(zeroStop==true && revState==WREWIND){

    //  int16_t stopPos=speedCorr(cnt, rewTab);
      if(mainCnt <=stopPos){
        revState=WSTOP;
        updateValue("status","STOP");
        runStop();
      }
    }
  }
