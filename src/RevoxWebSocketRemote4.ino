#include <Arduino.h>

#include <EEPROM.h>
#include <Hash.h>

#include "MCP_Code.h"
#include "MemoryCode.h"
#include "WEB_CODE.h"
#include "EncoderCode.h"

//#include "WifiSettings.h"
volatile int integerValue=0;

/*__________________________________________________________SETUP__________________________________________________________*/

void setup() {

  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println("\r\n");

  Serial.println("Revox Wifi Remote V0.9 beta 15.6.2017");
//  Serial.println(__FILE__);
  Serial.println("Compiled: " __DATE__ " " __TIME__);
  Serial.println("File:     RevoxWebSocketRemote.ino\n");


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

  updateCounter("maincnt",0);
  updateCounters();

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
  }

}
