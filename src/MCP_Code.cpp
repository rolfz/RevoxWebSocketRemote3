/*
This file contains the code for connecting the Z-Control
Revox WIFI interface to an MCP23017 chip.
It used the Adafruit driver and all related code is located
inside MCP_Code.cpp
All IO related details are located inside MCP_Code.h

By Rolf Ziegler
June 2017
Version 1.0
*/
//#define DEBUG // enable printout debug info
#include <Wire.h>
#include <Adafruit_MCP23017.h>

#include "MCP_Code.h"

Adafruit_MCP23017 mcp;

// Interrupts from the MCP will be handled by this PIN
byte mcpIntBPin=5; //D1;

// ... and this interrupt vector
byte interruptVector=1;

volatile boolean awakenByInterrupt = false;
volatile boolean pauseLock=false;

volatile boolean play=false;
volatile boolean fore=false;
volatile boolean reve=false;
volatile boolean rec=false;

void startMCP(void){
// setup i2c, Ada lib seems not to work without it.
  Wire.begin(I2C_SDA,I2C_SCL);
  Wire.setClock(400000);

// init interrupt line IO
  pinMode(mcpIntBPin,INPUT); // mcp interrupt pin on WemosD1

  // We will setup a pin for flashing from the int routine
  pinMode(LED1_PIN, OUTPUT);  // use the p13 LED as debugging
  pinMode(LED2_PIN, OUTPUT);  // use the p13 LED as debugging

  Serial.println("ESP Initalized");

// init mcp chip
  mcp.begin();      // use default address 0

  // No mirror we only need INTB interrupt as this is the only input port
  // The INTA/B will not be Floating
  // INTs will be signaled with a LOW
  mcp.setupInterrupts(false,false,LOW);
//  mcp.pullUp(INTBpin, HIGH);  // turn on a 100K pullup internall

  // configuration for a button on port A
  // interrupt will triger when the pin is taken to ground by a pushbutton
//  mcp.pullUp(mcpRCi, HIGH);  // turn on a 100K pullup internally

// initalize port A on MCP
  mcp.writeRegister(MCP23017_IODIRA,0x00);
  mcp.writeRegister(MCP23017_OLATA,0); // write 0 to output
  Serial.println("MPC PortA as Output initalized");

// initalize port B as Input with interrup
// input on B
  mcp.writeRegister(MCP23017_IOCONB,0x00); // Interrupt output is active Low
  mcp.writeRegister(MCP23017_IODIRB,0xff); // All Port B are input pins
  //mcp.writeRegister(MCP23017_DEFVALB,0x00); // Default value is zero
  //mcp.writeRegister(MCP23017_INTCONB,0xff); // All inputs are compared to default value
  mcp.writeRegister(MCP23017_INTCONB,0x00); // interrupt on change of a pin
  mcp.writeRegister(MCP23017_GPINTENB,0x3f);  // ALL B pins are able to trigger interrupt
  //mcp.setupInterruptPin(REV_PA_IN,RISING);
//  mcp.setupInterruptPin(REV_RE_IN,RISING);

  Serial.println("MPC PortB as Input initalized");

  attachInterrupt(mcpIntBPin, intCallBack, FALLING);
  Serial.println("ESP Interrupt activated");

}

// The int handler will just signal that the int has happen
// we will do the work from the main loop.
void intCallBack(){
  awakenByInterrupt=true;
}

void displayOutput(){
  #ifdef DEBUG
  Serial.print("Output is: 0x");
  Serial.println(mcp.readRegister(MCP23017_GPIOB),HEX);
  #endif
}

void runPlay(void)
{
    // first we avoid to get an other interrupt on Pause which may lock the chip
      mcp.writeRegister(MCP23017_GPINTENB,0x3f);  // ALL B pins are able to trigger interrupt
    // we delay until we send out the Pause signal. It did not work wihtout it.
      delay(10);
      mcp.digitalWrite(REV_PL_OUT, HIGH);
      delay(100);
      displayOutput();
      mcp.digitalWrite(REV_PL_OUT, LOW);
}

void runStop(void)
{
    // first we avoid to get an other interrupt on Pause which may lock the chip
      mcp.writeRegister(MCP23017_GPINTENB,0x3f);  // ALL B pins are able to trigger interrupt
    // we delay until we send out the Pause signal. It did not work wihtout it.
      delay(10);
      mcp.digitalWrite(REV_ST_OUT, HIGH);
      delay(100);
      displayOutput();
      mcp.digitalWrite(REV_ST_OUT, LOW);
}
void runRewind(void)
{
    // first we avoid to get an other interrupt on Pause which may lock the chip
      mcp.writeRegister(MCP23017_GPINTENB,0x3f);  // ALL B pins are able to trigger interrupt
    // we delay until we send out the Pause signal. It did not work wihtout it.
      delay(10);
      mcp.digitalWrite(REV_RE_OUT, HIGH);
      delay(100);
      displayOutput();
      mcp.digitalWrite(REV_RE_OUT, LOW);
}
void runForward(void)
{
    // first we avoid to get an other interrupt on Pause which may lock the chip
      mcp.writeRegister(MCP23017_GPINTENB,0x3f);  // ALL B pins are able to trigger interrupt
    // we delay until we send out the Pause signal. It did not work wihtout it.
      delay(10);
      mcp.digitalWrite(REV_FO_OUT, HIGH);
      delay(100);
      displayOutput();
      mcp.digitalWrite(REV_FO_OUT, LOW);
}
void runRecord(void)
{
    // first we avoid to get an other interrupt on Pause which may lock the chip
      mcp.writeRegister(MCP23017_GPINTENB,0x3f);  // ALL B pins are able to trigger interrupt
    // we delay until we send out the Pause signal. It did not work wihtout it.
      delay(10);
      mcp.digitalWrite(REV_PL_OUT, HIGH);
      mcp.digitalWrite(REV_RC_OUT, HIGH);
      displayOutput();
      delay(100);
      mcp.digitalWrite(REV_RC_OUT, LOW);
      mcp.digitalWrite(REV_PL_OUT, LOW);
}
void lockPause(void)
{
    // first we avoid to get an other interrupt on Pause which may lock the chip
      mcp.writeRegister(MCP23017_GPINTENB,0x1f);  // ALL B pins are able to trigger interrupt
    // we delay until we send out the Pause signal. It did not work wihtout it.
      delay(10);
      mcp.digitalWrite(REV_PA_OUT, HIGH);
      delay(10);
      displayOutput();
      pauseLock=true;
      #ifdef DEBUG
      Serial.println("Pause locked");
      #endif
}
void unlockPause(void)
{
      // unlock is easier, we remove the Pause signal
      mcp.digitalWrite(REV_PA_OUT, LOW);
      pauseLock=false;
      #ifdef DEBUG
      Serial.println("Pause unlocked");
      #endif
      delay(2);
      displayOutput();
      // finally we reinitalize the pause interrupt.
      mcp.writeRegister(MCP23017_GPINTENB,0x3f);  // ALL B pins are able to trigger interrupt
}


// MCP Interrupt Cleaner
void cleanInterrupts(){

  awakenByInterrupt=false;       // Resets int boolean to false
  attachInterrupt(mcpIntBPin, intCallBack, FALLING);
  digitalWrite(LED1_PIN,LOW);
}


// this routine is called once an external interrupt occured
// routine can also be called from the WEB interface without interrupt.
void handleInterrupt(){

  uint8_t pin;
  uint8_t val;
 // let's get the data from the source of the call
  if(wifiCall==false) {
  // Get more information from the MCP from the INT
  pin=mcp.getLastInterruptPin();
  val=mcp.getLastInterruptPinValue();
  }
  else {
  pin=wifiPin;
  val=1;
  }

  detachInterrupt(mcpIntBPin);

  digitalWrite(LED1_PIN,HIGH);

#ifdef DEBUG
  Serial.print("interrupt pin is: ");
  Serial.print(pin);
  Serial.print(" value is: ");
  Serial.println(val);
#endif
// let's act on the actions from the interrupt
  switch(pin){
    case RecordPin:
        #ifdef DEBUG
        Serial.println("Pressed RECORD");
        #endif
        if(val==1 && pauseLock==true){
               unlockPause();
               }
         runRecord();
         break;
    case StopPin:
          if(val==1) play=false; // we pressed stop
         #ifdef DEBUG
         Serial.println("Pressed STOP");
         #endif
         if(val==1 && pauseLock==true){
                    unlockPause();
                    }
         runStop();
         break;
    case PlayPin:
         if(val==1) play=true; // play and enable the pause function
         #ifdef DEBUG
         Serial.println("Pressed PLAY");
         #endif
         runPlay();
         if(val==1 && pauseLock==true){
                    unlockPause();
                    }
        runPlay();
         break;
    case RewindPin:
         #ifdef DEBUG
         Serial.println("Pressed REWIND");
         #endif
         if(val==1 && pauseLock==true){
                    unlockPause();
                    }
         runRewind();
         break;
    case ForwardPin:
         #ifdef DEBUG
         Serial.println("Pressed FOREWARD");
         #endif
         // if one of this buttons pressesd, we release Pause
           if(val==1 && pauseLock==true){
                    unlockPause();
                    }
          runForward();
           break;
    case PausePin:
          #ifdef DEBUG
          Serial.println("Pressed PAUSE");
          Serial.print("pause: "); Serial.print( val); Serial.print(" ");
          Serial.println(pauseLock);
          #endif
           if(val==1 && pauseLock==false){
                lockPause();
                }
         break;
    case REV_PA_IN+REV_RE_IN:
          #ifdef DEBUG
          Serial.println("Pressed RECORD");
          #endif
          if(val==1 && pauseLock==false){
                lockPause();
                }
             runRecord();
         break;
// encoder code
/*
    case EncDir:
    break;
    case EncSens:
    break;
*/

    default:
    break;
    val=0;
  }

// release interrupt by reading the 2 ports
   mcp.readRegister(MCP23017_GPIOB);
   #ifdef DEBUG
   Serial.print("Output is: 0x");
   Serial.println(mcp.readRegister(MCP23017_GPIOB),HEX);
   #endif
  // and clean queued INT signal
   cleanInterrupts();
   // interrupt service done
}
