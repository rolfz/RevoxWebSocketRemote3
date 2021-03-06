/*
    Revox B77 memory code
    Stores values of 3-5 segments of tape to be played

    By Rolf Ziegler
    June-Juillet 2017

    Input: functions called from the web interface to store and play the segments
    Output: functions called from the web interface to goto or play segments of recorded material

*/
//#define DEBUG

#include "MemoryCode.h"
#include "autoPlay.h"

// memory map to store data into EEPROM
// location
//    0-1    int (2bytes) "mainCnt" stored at power down
#define MAINCNT_START 0
//    2-19   int "start + end" locations each 2 bytes * 5, only 2 used for now
#define MEMORY_START 2
//   20-29   int "offset" forward/reverse each 2 bytes * 5, all used
#define OFFSET_START 22

volatile int16_t cntS[5]={0,0,0,0,0};
volatile int16_t cntE[5]={0,0,0,0,0};

// initalize eeprom function
void startEEPROM(void){

    EEPROM.begin(512);

//    dumpMemory();
}

// store offset from the web page
// we have 5x 2 values of 16 bits =20 bytes
void storeOffset(const char * index, int16_t value){

// storeage of one integer takes 2 bytes
// as memory name starts with 1, we need to deduct 2 from the OFFSET_START
  int8_t loc=(index[2]-0x30);
  int8_t memloc=loc*2+OFFSET_START-2;

    if(index[1]=='B'){ // it is foreward
      memloc+=12; // this is to store the backwards data
      rewTab[loc-1]=value; // we need to deduc 1 as memory for 1 is loc=0
      }
      else {
      forTab[loc-1]=value;
      }
#ifdef DEBUG
      Serial.print("Writing:");
      Serial.print(value);
      Serial.print("to: ");
      Serial.print(loc);
      Serial.print("web: ");
      Serial.println(index);
#endif
  EEPROM.write(memloc, value & 0xff);
  EEPROM.write(memloc+1, value>>8);
  EEPROM.commit();
}

// restore values into .... at start
//extern int rewTab[];
//extern int forTab[];

void restoreOffset(void){
  for(int loc=0;loc<6;loc++){
    // loc starts et 2 in memory (0-1 are used by mainCnt)
    int memLoc=loc*2+OFFSET_START;
// restore forward
    int16_t tmp=EEPROM.read(memLoc);
    tmp+=EEPROM.read(memLoc+1)<<8;
    forTab[loc]=tmp;
#ifdef DEBUG
    Serial.print("Reading:");
    Serial.print(forTab[loc]);
    Serial.print("from: ");
    Serial.println(memLoc);
#endif
// restore reverse
    tmp=EEPROM.read(memLoc+12);
    tmp+=EEPROM.read(memLoc+13)<<8;
    rewTab[loc]=tmp;
#ifdef DEBUG
    Serial.print("Reading:");
    Serial.print(rewTab[loc]);
    Serial.print("from: ");
    Serial.println(memLoc+12);
#endif
  }
}

// store one position of the autoplay locations
int16_t storePosition(uint8_t * index, int16_t cnt){

  int8_t loc=index[2]-'0'; // will be 1/2/3 (*2 means start at 2, no offset needed)
  if(index[1]=='S'){
             cntS[loc-1]=cnt;
             loc=loc*2; // memory location
             }
        else{ // it is an 'E'
             cntE[loc-1]=cnt;
             loc=(loc*2)+10; // memory location
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

    #ifdef DEBUG
    int tmp = EEPROM.read(loc);
    tmp +=EEPROM.read(loc+1)<<8;
    Serial.print(" read loc: ");
    Serial.print(loc);
    Serial.print(" = ");
    Serial.println(tmp);
    #endif
    return(cnt);
    }

//
void storeMainCnt(void){
    int16_t cnt=mainCnt;
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
    for(int i=0;i<3;i++){
      int loc=i*2+MEMORY_START;

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
      cntE[i]=EEPROM.read(loc+10);
      cntE[i]+=EEPROM.read(loc+11)<<8;
    #ifdef DEBUG
    Serial.print("restored cntE");
    Serial.print(loc);
    Serial.print(" = ");
    Serial.println(cntE[i]);
    #endif
    }
}

  void dumpMemory(void){
    for(int i=0;i<26;i++){
      Serial.print("loc: ");
      Serial.print(i);
      Serial.print(" = ");
      Serial.println(EEPROM.read(i));
    }
  }
