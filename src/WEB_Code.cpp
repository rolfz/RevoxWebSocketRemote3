/*
  WEB code for RevoxB77 wifi remote controller

  By Rolf Ziegler
  June 2017

/*________________________________________________PAYLOAD CODE__________________________________________________________*/
#include "WEB_Code.h"
#include "MemoryCode.h"
#include "WifiSettings.h"
#include "MCP_Code.h"

//#define DEBUG

ESP8266WiFiMulti wifiMulti;       // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server = ESP8266WebServer(80);       // create a web server on port 80 -99
WebSocketsServer webSocket = WebSocketsServer(81);    // create a websocket server on port 81

File fsUploadFile;                                    // a File variable to temporarily store the received file


volatile int mainCnt=0; // Tape counter

enum  revstat {STOP,PLAY,FORWARD,REWIND,RECORD,PAUSE,NOPAUSE};
char  revState=STOP;
char pauseState=NOPAUSE;

const char* mdnsName = "esp8266"; // Domain name for the mDNS responder

char json[10000];                                   // Buffer pour export du JSON - JSON export buffer

volatile bool wifiCall = false;
volatile uint8_t wifiPin = 0;

void updateValue( String(id),String (data)) {

  String json = "{\"id\": \"" + id + "\",";
         json+= "\"value\": \"" + data + "\"}";

//  Serial.print("JSON : ");  Serial.println(json);

      webSocket.broadcastTXT(json);
}

void updateCounter( String(id),int (data)) {

  String json = "{\"id\": \"" + id + "\",";
         json+= "\"count\": \"" + String(data) + "\"}";

//  Serial.print("JSON : ");  Serial.println(json);

      webSocket.broadcastTXT(json);
}

// initial counter update
void updateCounters() {

 String json = "{\"c1s\": \"" + String(cntS[1]) + "\",";   // pack all other memory values
        json += "\"c1e\": \"" + String(cntE[1]) + "\",";
        json += "\"c2s\": \"" + String(cntS[2]) + "\",";
        json += "\"c2e\": \"" + String(cntE[2]) + "\",";
        json += "\"c3s\": \"" + String(cntS[3]) + "\",";
        json += "\"c3e\": \"" + String(cntE[3]) + "\",";
        json += "\"c4s\": \"" + String(cntS[4]) + "\",";
        json += "\"c4e\": \"" + String(cntE[4]) + "\",";
        json += "\"c5s\": \"" + String(cntS[5]) + "\",";
        json += "\"c5e\": \"" + String(cntE[5]) + "\"}";


  server.send(200, "application/json", json);
  // Serial.print("Counter ");  Serial.print(mainCnt);  Serial.println(" updated");
}


/*__________________________________________________________SETUP_FUNCTIONS__________________________________________________________*/

void startWiFi() { // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection

  WiFi.disconnect();
  WiFi.softAPdisconnect();
  WiFi.setAutoConnect(true);

  WiFi.softAP(ssid, password);             // Start the access point
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started\r\n");

  wifiMulti.addAP(myssid, mypassword);   // add Wi-Fi networks you want to connect to
  wifiMulti.addAP(ssid2, password2);
  wifiMulti.addAP(ssid3, password3);

  Serial.println("Connecting");
  while (wifiMulti.run() != WL_CONNECTED && WiFi.softAPgetStationNum() < 1) {  // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }
  Serial.println("\r\n");
  if (WiFi.softAPgetStationNum() == 0) {     // If the ESP is connected to an AP
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());             // Tell us what network we're connected to
    Serial.print("IP address:\t");
    Serial.print(WiFi.localIP());            // Send the IP address of the ESP8266 to the computer
  } else {                                   // If a station is connected to the ESP SoftAP
    Serial.print("Station connected to ESP8266 AP");
  }
  Serial.println("\r\n");
}

void startOTA() { // Start the OTA service
  // Port defaults to 8266
  ArduinoOTA.setPort(8081);

  ArduinoOTA.setHostname(OTAName);
  ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");

    // ADD INIT LED HERE

  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\r\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready\r\n");
/*  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());*/
}

void startSPIFFS() { // Start the SPIFFS and list all contents
  SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
  Serial.println("SPIFFS started. Contents:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {                      // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
}

void startWebSocket() { // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void startMDNS() { // Start the mDNS responder
  MDNS.begin(mdnsName);                        // start the multicast domain name server
  Serial.print("mDNS responder started: http://");
  Serial.print(mdnsName);
  Serial.println(".local");
}


void startServer() { // Start a HTTP server with a file read handler and an upload handler
  server.on("/edit.html",  HTTP_POST, []() {  // If a POST request is sent to the /edit.html address,
    server.send(200, "text/plain", "");
  }, handleFileUpload);                       // go to 'handleFileUpload'

  server.on("/update.json", updateCounters);
  server.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'
  // and check if the file exists


  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/css", SPIFFS, "/css");
  server.serveStatic("/img", SPIFFS, "/img");
  server.serveStatic("/", SPIFFS, "/index.html");

  server.begin();                             // start the HTTP server
  Serial.println("HTTP server started.");
}

/*__________________________________________________________SERVER_HANDLERS__________________________________________________________*/

void handleNotFound() { // if the requested file or page doesn't exist, return a 404 not found error
  if (!handleFileRead(server.uri())) {        // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  #ifdef DEBUG
  Serial.println("handleFileRead: " + path);
  #endif
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    #ifdef DEBUG
    Serial.println(String("\tSent file: ") + path);
    #endif
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

void handleFileUpload() { // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  String path;
  if (upload.status == UPLOAD_FILE_START) {
    path = upload.filename;
    if (!path.startsWith("/")) path = "/" + path;
    if (!path.endsWith(".gz")) {                         // The file server always prefers a compressed version of a file
      String pathWithGz = path + ".gz";                  // So if an uploaded file is not compressed, the existing compressed
      if (SPIFFS.exists(pathWithGz))                     // version of that file must be deleted (if it exists)
        SPIFFS.remove(pathWithGz);
    }
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    path = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {                                   // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location", "/success.html");     // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length ) { // When a WebSocket message is received

  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      #ifdef DEBUG
      Serial.printf("[%u] get command: %s\n", num, payload);
      #endif
      if (payload[0] == '#') {           // We received a key press from the web page

        // convert web command into MCP interrupt call.
        switch (payload[1]) {
          case 'P': wifiPin = PausePin;
                    pauseState=PAUSE;
                    updateValue("status","PAUSE");
            break;
          case 'R': wifiPin = RewindPin;
                    revState=REWIND;
                    pauseState=NONE;
                    updateValue("status","REWIND");
            break;
          case 'F': wifiPin = ForwardPin;
                    revState=FORWARD;
                    pauseState=NONE;
                    updateValue("status","FORWARD");
            break;
          case 'L': wifiPin = PlayPin;
                    revState=PLAY;
                    pauseState=NONE;
                    updateValue("status","PLAY");
            break;
          case 'S': wifiPin = StopPin;
                    revState=STOP;
                    pauseState=NONE;
                    updateValue("status","STOP");
            break;
          case 'E': wifiPin = RecordPin;
                    revState=RECORD;
                    pauseState=NONE;
                    updateValue("status","RECORD");
            break;
          default: wifiPin = NONE;
        }
        // if we have a valid command we call interrupt and pass the command to the MPC code.
        if (wifiPin != NONE) {
          wifiCall = true;
          handleInterrupt();
        }
        wifiCall = false;
      }


      if (payload[0] == '$') {           // We received a key press from the web page
        /* $ commands are used to control the memory locations of the tape recorder
             format: $S  $G $E $O $P
                 for: S= Store Start
                      G= Goto Start
                      E= Store End position
                      O= Goto End position
                      P= Play the programmed part        /*
        Serial.print(String(payload[1]));
        Serial.print("memory: ");
        Serial.println(String(payload[2]));
        */
        char str[10];
        memcpy(str, payload,length);
        str[length] = '\0';
        str[0]='p';

        switch(payload[1]) {
              case 'S': case 'E': storePosition(payload);
                                updateCounter(str,mainCnt);
                               break;
              case 'G':   gotoPosition(payload);
                          break;
              case 'O':   gotoPosition(payload);
                          break;
              case 'P':   playMemory(payload[2]);
                          break;

              case 'C':   mainCnt=0;
                          storeMainCnt();
                          #ifdef DEBUG
                          Serial.println("Counter cleared");
                          #endif
                          break;
                        }
//        updateCounters();
      }
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);

      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

/*__________________________________________________________HELPER_FUNCTIONS__________________________________________________________*/

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}

String getContentType(String filename) { // determine the filetype of a given filename, based on tohe extension
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
