#ifndef __MCPCODE__
#define __MCPCODE__
/*




*/
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Arduino.h>

// constants
byte const LED1_PIN=D7;
byte const LED2_PIN=D8;

// ESP pin definitions

byte const I2C_SDA=2; //D4;
byte const I2C_SCL=0; //D3; 
byte const HALL_SENS= 4; //D2;
byte const HALL_DIR=  12; //D6;

// Revox output signals on port A
byte const REV_PA_OUT=0;
byte const REV_RE_OUT=1;
byte const REV_FO_OUT=2;
byte const REV_PL_OUT=3;
byte const REV_ST_OUT=4;
byte const REV_RC_OUT=5;
byte const REV_SL_OUT=6;

// Revox input signals on port B
//byte const REV_RC_IN=1;
byte const REV_RC_IN=8;
byte const REV_ST_IN=9;
byte const REV_PL_IN=10;
byte const REV_FO_IN=11;
byte const REV_RE_IN=12;
byte const REV_PA_IN=13; // for testing
byte const REV_DIR_IN=14;
byte const REV_SENS_IN=15;

#define RecordPin 8
#define StopPin 9
#define PlayPin 10
#define ForwardPin 11
#define RewindPin 12
#define PausePin 13
#define NONE 0

// prototypes
void intCallBack();
// call interrup from wifi input
extern volatile bool wifiCall;
extern volatile uint8_t  wifiPin;
extern volatile boolean awakenByInterrupt;

void startMCP(void);
void intCallBack();

void lockPause(void);
void unlockPause(void);
void cleanInterrupts();
void handleInterrupt();
  
#endif  
