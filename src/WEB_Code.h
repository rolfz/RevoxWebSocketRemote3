#ifndef __WEB_CODE__
#define __WEB_CODE__

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WebSocketsServer.h>

extern ESP8266WiFiMulti wifiMulti;
extern ESP8266WebServer server;
extern WebSocketsServer webSocket;

extern File fsUploadFile;
/*________________________________________________PAYLOAD CODE__________________________________________________________*/

void updateValue( String(id),String (data));
void updateCounter( String(id),int (data));
void updateCounters(void);
void updateOffsets(void);
/*__________________________________________________________SETUP_FUNCTIONS__________________________________________________________*/

void startWiFi();
void startOTA();
void startSPIFFS();
void startWebSocket();
void startMDNS();
void startServer();
/*__________________________________________________________SERVER_HANDLERS__________________________________________________________*/

void handleNotFound();
bool handleFileRead(String path) ;
void handleFileUpload();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length ) ;

/*__________________________________________________________HELPER_FUNCTIONS__________________________________________________________*/

String formatBytes(size_t bytes) ;
String getContentType(String filename);
#endif
