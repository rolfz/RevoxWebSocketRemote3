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

void lockPause(void)
{
    // first we avoid to get an other interrupt on Pause which may lock the chip
      mcp.writeRegister(MCP23017_GPINTENB,0x1f);  // ALL B pins are able to trigger interrupt
    // we delay until we send out the Pause signal. It did not work wihtout it.
      delay(10);
      mcp.digitalWrite(REV_PA_OUT, HIGH);
      delay(10);
      pauseLock=true;
      Serial.println("Pause locked");
}
void unlockPause(void)
{
      // unlock is easier, we remove the Pause signal
      mcp.digitalWrite(REV_PA_OUT, LOW);
      pauseLock=false;
      Serial.println("Pause unlocked");
      delay(2);
      // finally we reinitalize the pause interrupt.
      mcp.writeRegister(MCP23017_GPINTENB,0x2f);  // ALL B pins are able to trigger interrupt
}


// MCP Interrupt Cleaner
void cleanInterrupts(){

  awakenByInterrupt=false;       // Resets int boolean to false
  attachInterrupt(mcpIntBPin, intCallBack, FALLING);
  digitalWrite(LED1_PIN,LOW);
}


// this routine is called once an external interrupt occured
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

         Serial.println("Pressed REC");

         break;
    case StopPin:
          if(val==1) play=false; // we pressed stop

         Serial.println("Pressed STOP");
         if(val==1 && pauseLock==1){
                    unlockPause();
                    }

         break;
    case PlayPin:
          if(val==1) play=true; // play and enable the pause function
         Serial.println("Pressed PLAY");
         if(val==1 && pauseLock==1){
                    unlockPause();
                    }
         break;
    case RewindPin:
         Serial.println("Pressed REWIND");
         if(val==1 && pauseLock==1){
                    unlockPause();
                    }
         break;
    case ForwardPin:
         Serial.println("Pressed FOREWARD");
         // if one of this buttons pressesd, we release Pause
           if(val==1 && pauseLock==1){
                    unlockPause();
                    }
           break;
    case PausePin:
             Serial.println("Pressed PAUSE");
           if(val==1 && pauseLock==0){
                lockPause();
                }
         break;
    case REV_PA_IN+REV_RE_IN:
         break;
    default:
    break;
    val=0;
  }

// release interrupt by reading the 2 ports
   mcp.readRegister(MCP23017_GPIOB);
   Serial.print("Output is: 0x");
   Serial.println(mcp.readRegister(MCP23017_GPIOB),HEX);
  // and clean queued INT signal
   cleanInterrupts();
   // interrupt service done
}
