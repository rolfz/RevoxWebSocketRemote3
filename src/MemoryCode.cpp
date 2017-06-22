/*



*/
#include "MemoryCode.h"

volatile int cntS[5]={0,0,0,0,0};
volatile int cntE[5]={0,0,0,0,0};

void startEEPROM(void){

    EEPROM.begin(512);
//    dumpMemory();
    restorePositions();
}

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

    Serial.print("Stored position ");
    Serial.print(loc);
    Serial.print("= ");
    Serial.print(cnt);

    int tmp = EEPROM.read(loc);
    tmp +=EEPROM.read(loc+1)<<8;
    Serial.print(" read loc: ");
    Serial.print(loc);
    Serial.print(" = ");
    Serial.println(tmp);
    }

void storeMainCnt(void){
    int cnt=mainCnt;
    EEPROM.write(0, cnt & 0xff);
    EEPROM.write(1, cnt >> 8);
    Serial.println("mainCnt stored");

    EEPROM.commit();
}

void restorePositions(){

    // restore counter
    mainCnt = EEPROM.read(0);
    mainCnt +=EEPROM.read(1)<<8;
    Serial.print("restored mainCnt =");
    Serial.println(mainCnt);

    // restore memory counters
    for(int i=1;i<=3;i++){
      int loc=i*2;

      // start positions
      // location 2,4,6 and 3,5,7
      cntS[i]=EEPROM.read(loc);
      cntS[i]+= EEPROM.read(loc+1)<<8;

    Serial.print("restored cntS");
    Serial.print(loc);
    Serial.print(" = ");
    Serial.println(cntS[i]);

      // end positions
      // location 12,14,16 and 13,15,17
      cntE[i]=EEPROM.read(loc+8);
      cntE[i]+=EEPROM.read((loc)+9)<<8;

    Serial.print("restored cntE");
    Serial.print(loc);
    Serial.print(" = ");
    Serial.println(cntE[i]);
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
