/*
  RevoxWebSocketRemote   Wifi Controller for REVOXB77 Tape recorder

  - Scans changes on Revox Remote Controller plug (inside recorder)
  - Generates a web page with Remote control buttons
  - Allows to store 3 samples on tape to be played indivisually
  - Clear button
  - Tape counter with hall detector/encoder
  - UART data to display counter on front display
  -

  By Rolf Ziegler
  June 2017
*/
#include <Arduino.h>

#include <EEPROM.h>
#include <Hash.h>

#include "MCP_Code.h"
#include "MemoryCode.h"
#include "WEB_CODE.h"
#include "EncoderCode.h"

//#include "WifiSettings.h"
volatile int integerValue=0;
int const SHUT_DOWN_PIN=14;
int const SHUT_DOWN_THESHOLD=1;

/*__________________________________________________________SETUP__________________________________________________________*/

void setup() {

  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println("\r\n");

  Serial.println("Revox Wifi Remote V0.94 beta 22.6.2017");
//  Serial.println(__FILE__);
  Serial.println("Compiled: " __DATE__ " " __TIME__);
  Serial.println("File:     RevoxWebSocketRemote.ino\n");

  pinMode(SHUT_DOWN_PIN,INPUT);

  mainCnt = 0;

  startMCP();

  startWiFi();                 // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection

  startOTA();                  // Start the OTA service

  startSPIFFS();               // Start the SPIFFS and list all contents

  startWebSocket();            // Start a WebSocket server

  startMDNS();                 // Start the mDNS responder

  startServer();               // Start a HTTP server with a file read handler and an upload handler

  startEEPROM();

  startEncoder();             // encoder handled by ESP

  updateCounters();           // restore the tape positions (autoplay)

  restoreCounters();           // restore counter position from last shut-down

}

/*__________________________________________________________LOOP__________________________________________________________*/

void loop() {
  static long blink = 0;
  static bool toggle = false;

  webSocket.loop();                           // constantly check for websocket events
  server.handleClient();                      // run the server

  ArduinoOTA.handle();                        // listen for OTA events

  // everything is done in the interrupt handling routine.
  if (awakenByInterrupt) handleInterrupt();

  if (blink++ >= 6000) {
    blink = 0;
    toggle = !toggle;
    digitalWrite(LED2_PIN, toggle);
  //  if(mainCnt++ > 9999)mainCnt=0;
      updateCounter("maincnt",mainCnt); // << we send the counter value here
     //Serial.print("!");
     //Serial.println(mainCnt);

     // test if Revox main power was turned off -> save main counter position
     int pwrLevel=digitalRead(SHUT_DOWN_PIN);

     #ifdef DEBUG
     Serial.print("Revox Power =");
     Serial.println(pwrLevel);
     #endif

         if(pwrLevel < SHUT_DOWN_THESHOLD){
            storeMainCnt();
            Serial.println("STORE MAIN COUNTER");
            while(1);
         }

      } // end blink and shut-down test

  // check if commands where received from front panel display

  if (Serial.available() > 0) {   // something came across serial

              char incomingByte = Serial.read();

              if (incomingByte == 'R'){
                  mainCnt=0;
                  updateCounter("maincnt",mainCnt);
                  }
                }

}// end main loop
