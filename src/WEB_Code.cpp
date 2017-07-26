/*
  WEB code for RevoxB77 wifi remote controller
  By Rolf Ziegler
  June-Juillet 2017
/*________________________________________________PAYLOAD CODE__________________________________________________________*/
#include "WEB_Code.h"
#include "MemoryCode.h"
#include "WifiSettings.h"
#include "MCP_Code.h"
#include "autoplay.h"
#include "ArduinoJson.h"

//#define DEBUG
#define MULTI

#ifdef MULTI
ESP8266WiFiMulti wifiMulti;       // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
#endif
ESP8266WebServer server = ESP8266WebServer(80);       // create a web server on port 80 -99
WebSocketsServer webSocket = WebSocketsServer(81);    // create a websocket server on port 81

File fsUploadFile;                                    // a File variable to temporarily store the received file


#define SERIAL_DEBUG  Serial

volatile char  revState=WSTOP;
char pauseState=WNOPAUSE;

const char* mdnsName = "esp8266"; // Domain name for the mDNS responder

char json[10000];                                   // Buffer pour export du JSON - JSON export buffer

volatile bool wifiCall = false;
volatile uint8_t wifiPin = 0;

/*__________________________________________________________SETUP_FUNCTIONS__________________________________________________________*/

void startWiFi() { // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
//#define INIT_WIFI
#ifdef INIT_WIFI
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_AP_STA);
 #endif
#define TESTING
  #ifdef TESTING
  bool autoConnect;
  WiFi.softAPdisconnect();

  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA); // softap only WIFI_AP_STA for both
  autoConnect = WiFi.getAutoConnect();
  #endif
  WiFi.softAP(ssid,password);             // Start the access point
//  WiFi.softAP(ssid);             // Start the access point

  Serial.print("Access Point \"");
  //Serial.print(ssid);
  Serial.print(ssid);
  Serial.println("\" started\r\n");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

#ifdef MULTI
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
#endif // MULTI
}

void startOTA() { // Start the OTA service
  // Port defaults to 8266
  //ArduinoOTA.setPort(8081);
  ArduinoOTA.setPort(8266);

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
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
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
  server.on("/offset.json", updateOffsets);

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
  #ifdef DEBUG
  // this line makes problems sending error on UART
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  #endif
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

//  Serial.print("type=");
//  Serial.println(type);

  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
      //  Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        updateCounter("mainCnt",mainCnt); // << we send the counter value here
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
                    pauseState=WPAUSE;
                    // update value will display the last command on the top line of the web page (grey)
                    updateValue("status","PAUSE");
            break;
          case 'R': wifiPin = RewindPin;
                    revState=WREWIND;
                    pauseState=NONE;
                    updateValue("status","REWIND");
                    // we prepare the stop position with correction. Stop in main loop
                    stopPos=speedCorr(mainCnt, rewTab );
            break;
          case 'F': wifiPin = ForwardPin;
                    revState=WFORWARD;
                    pauseState=NONE;
                    updateValue("status","FORWARD");
            break;
          case 'L': wifiPin = PlayPin;
                    revState=WPLAY;
                    pauseState=NONE;
                    updateValue("status","PLAY");
            break;
          case 'S': wifiPin = StopPin;
                    revState=WSTOP;
                    pauseState=NONE;
                    updateValue("status","STOP");
                    task=WSTOP;
            break;
          case 'E': wifiPin = RecordPin;
                    revState=WRECORD;
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
      } // end #


      if (payload[0] == '$') {           // We received a key press from the web page
        /* $ commands are used to control the memory locations of the tape recorder
             format: $S  $G $E $O $P
                 for: S= Store Start
                      G= Goto Start
                      E= Store End position
                      O= Goto End position
                      P= Play the programmed part
                      R= repeat, set variable true or false
        Serial.print(String(payload[1]));
        Serial.print("memory: ");
        Serial.println(String(payload[2]));
        */

        // create name for sending data to the web page
        // $ is replaced with m
        char str[10];
        memcpy(str, payload,length);
        str[length] = '\0';
        str[0]='n';

        switch(payload[1]) {
              // store start and end position
              case 'S': case 'E':{
                              int16_t tmpCnt=storePosition(payload,mainCnt);
                              //  restoreCounters(); // we could add code to update individual counters
                              updateCounter(str,tmpCnt);
                              //updateCounter(str,tmpCnt);
                               }
                               break;
              // goto start position
              case 'G':   gotoPosition(payload);
                          break;
              // goto end position
              case 'O':   gotoPosition(payload);
                          break;
              // play memory location
              case 'P':   playMemory(payload);
                          break;
              // clear mainCnt
              case 'C':   mainCnt=0;
                          storeMainCnt();
                          #ifdef DEBUG
                          Serial.println("Counter cleared");
                          #endif
                          updateCounter(str,mainCnt);
                          break;
              case 'Z':
                          gotoZero();
                          break;
                        }

          payload[0]='X';
      } // end $

      // we handle data from the tape delay page (setup) in json format
      // or the new counter value if the id is newCnt
      if(payload[0]=='{'){ // we get json in the payload

        StaticJsonBuffer<300> JSONBuffer;   //Memory pool
        JsonObject& parsed = JSONBuffer.parseObject(payload); //Parse

        if (!parsed.success()) {

          Serial.println("Parsing failed");
          return;
        }

        const char * id = parsed["id"]; //Get sensor type value
        int16_t value = parsed["value"];                                         //Get value of sensor measurement

        if(String(id)=="newCnt"){
           mainCnt=value;
           // update done in main loop.
        }
        else

        // change memory location.
        if(String(id[0])=="n"){

          storePosition((uint8_t *)id,value);
          #ifdef DEBUG
          Serial.println("received Memory counter");
          Serial.print("ID: ");
          Serial.print(id);
          Serial.print(" value: ");
          Serial.println(value);
          #endif
          updateCounter(String(id),value);
        //  updateCounters();
        } else

        if(String(id[0])=="o"){
          #ifdef DEBUG
          Serial.println("received offset");
          Serial.print("ID: ");
          Serial.print(id);
          Serial.print(" value: ");
          Serial.println(value);
          #endif
          storeOffset(id,value);
//          updateOffsets();
          //updateValue(String(id), String (value));
        }
        else {
          Serial.println("error: unknown command from web page!");
        }
      }

      if(payload[0]=='%'){           // We received a key press from the web page

    //  Serial.printf("get command: %s\n", payload);

          if(payload[1]=='0'){
              zeroStop=false;
              }
              else{
              zeroStop=true;
              }
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

/*--------------------------------------- Counter update ------------------------------------*/
// routine to update a field of text
void updateValue( String(id),String (data)) {

  String json = "{\"id\": \"" + id + "\",";
         json+= "\"value\": \"" + data + "\"}";
//  Serial.print("JSON : ");  Serial.println(json);
      webSocket.broadcastTXT(json);
}
// routine to update a number converted into a string
// update main counter
void updateCounter( String(id),int16_t (data)) {

  String json = "{\"id\": \"" + id + "\",";
         json+= "\"count\": \"" + String(data) + "\"}";
//  Serial.print("JSON : ");  Serial.println(json);
      webSocket.broadcastTXT(json);
}

// initial counter update
void updateCounters() {

 String json = "{\"mS1\": \"" + String(cntS[0]) + "\",";   // pack all other memory values
        json += "\"mE1\": \"" + String(cntE[0]) + "\",";
        json += "\"mS2\": \"" + String(cntS[1]) + "\",";
        json += "\"mE2\": \"" + String(cntE[1]) + "\",";
        json += "\"mS3\": \"" + String(cntS[2]) + "\",";
        json += "\"mE3\": \"" + String(cntE[2]) + "\"}";
/*
  Serial.print("Counters: ");
  Serial.println(json);
*/
  server.send(200, "application/json", json);
  json=" ";
  // Serial.print("Counter ");  Serial.print(mainCnt);  Serial.println(" updated");
}


// initial counter update for the tape stop delays (offsets)
void updateOffsets() {

 String json = "{\"o1b\": \"" + String(rewTab[0]) + "\",";   // pack all other memory values
        json += "\"o1f\": \"" + String(forTab[0]) + "\",";
        json += "\"o2b\": \"" + String(rewTab[1]) + "\",";
        json += "\"o2f\": \"" + String(forTab[1]) + "\",";
        json += "\"o3b\": \"" + String(rewTab[2]) + "\",";
        json += "\"o3f\": \"" + String(forTab[2]) + "\",";
        json += "\"o4b\": \"" + String(rewTab[3]) + "\",";
        json += "\"o4f\": \"" + String(forTab[3]) + "\",";
        json += "\"o5b\": \"" + String(rewTab[4]) + "\",";
        json += "\"o5f\": \"" + String(forTab[4]) + "\",";
        json += "\"o6b\": \"" + String(rewTab[5]) + "\",";
        json += "\"o6f\": \"" + String(forTab[5]) + "\"}";

//Serial.print("Offset: ");
//Serial.println(json);
  server.send(200, "application/json", json);
  json=" ";
  // Serial.print("Counter ");  Serial.print(mainCnt);  Serial.println(" updated");
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

// end of code
