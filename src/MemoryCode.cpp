F/*
    Revox B77 memory code
    Stores values of 3-5 segments of tape to be played

    By Rolf Ziegler
    June 2017

    Input: functions called from the web interface to store and play the segments
    Output: functions called from the web interface to goto or play segments of recorded material

*/
//#define DEBUG

#include "MemoryCode.h"

volatile int cntS[5]={0,0,0,0,0};
volatile int cntE[5]={0,0,0,0,0};

// initalize eeprom function
void startEEPROM(void){

    EEPROM.begin(512);

    dumpMemory();
}

// store one position of the autoplay locations
void storePosition(uint8 * index){

  int cnt=mainCnt; // we store not to loose the exact position
  int loc=index[2]-'0'; // will be 1/2/3
  if(index[1]=='S'){
             cntS[loc]=cnt;
             loc=loc*2; // memory location
             }
        else{
             cntE[loc]=cnt;
             loc=(loc*2)+8; // memory location
            }

    EEPROM.write(loc, cnt & 0xff);
    EEPROM.write(loc+1, cnt>>8);

    EEPROM.commit();

    #ifdef DEBUG
    Serial.print("Stored position ");
    Serial.print(loc);
    Serial.print("= ");
    Serial.print(cnt);
    #endif

    int tmp = EEPROM.read(loc);
    tmp +=EEPROM.read(loc+1)<<8;
    #ifdef DEBUG
    Serial.print(" read loc: ");
    Serial.print(loc);
    Serial.print(" = ");
    Serial.println(tmp);
    #endif
    }

//
void storeMainCnt(void){
    int cnt=mainCnt;
    EEPROM.write(0, cnt & 0xff);
    EEPROM.write(1, cnt >> 8);
    #ifdef DEBUG
    Serial.println("mainCnt stored");
    #endif
    EEPROM.commit();
}

void restoreCounters(){

    // restore counter
    mainCnt = EEPROM.read(0);
    mainCnt +=EEPROM.read(1)<<8;
    #ifdef DEBUG
    Serial.print("restored mainCnt =");
    Serial.println(mainCnt);
    #endif
    // restore memory counters
    for(int i=1;i<=3;i++){
      int loc=i*2;

      // start positions
      // location 2,4,6 and 3,5,7
      cntS[i]=EEPROM.read(loc);
      cntS[i]+= EEPROM.read(loc+1)<<8;
    #ifdef DEBUG
    Serial.print("restored cntS");
    Serial.print(loc);
    Serial.print(" = ");
    Serial.println(cntS[i]);
    #endif
      // end positions
      // location 12,14,16 and 13,15,17
      cntE[i]=EEPROM.read(loc+8);
      cntE[i]+=EEPROM.read((loc)+9)<<8;
    #ifdef DEBUG
    Serial.print("restored cntE");
    Serial.print(loc);
    Serial.print(" = ");
    Serial.println(cntE[i]);
    #endif
    }
}

void gotoPosition(uint8 * index){

  int loc=index[2]-'0';

  int gotoPos=EEPROM.read(loc);
      gotoPos+=EEPROM.read(loc+1)<<8;
  // add code to goto here
  }

void playMemory(uint8 loc){

  int startPos=EEPROM.read(loc);
      startPos+=EEPROM.read(loc+1)<<8;

  int endPos=EEPROM.read(loc+8);
      endPos+=EEPROM.read(loc+9)<<8;
  }

  void dumpMemory(void){
    for(int i=0;i<26;i++){
      Serial.print("loc: ");
      Serial.print(i);
      Serial.print(" = ");
      Serial.println(EEPROM.read(i));
    }
  }
