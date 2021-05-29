// http://librarymanager/All#M5StickC https://github.com/m5stack/M5StickC
//#define M5STICKCPLUS
#ifdef M5STICKCPLUS
#include <M5StickCPlus.h>
#else
#include <M5StickC.h>
#endif
#include <Preferences.h>
// WiFi
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WebServer.h>
#include <Update.h>
#include <WiFiUdp.h>
// ATEM
// https://github.com/kasperskaarhoj/SKAARHOJ-Open-Engineering/tree/master/ArduinoLibs
#include <SkaarhojPgmspace.h> 
#include <ATEMbase.h>
#include <ATEMstd.h>
// MQTT
//#include <SPI.h>
#include <PubSubClient.h> 

#define LED_PIN 10
#define LED_HAT_RED 0
#define LED_HAT_GREEN 26
#define LED_HAT_INPUT 36

// c:\Software\Arduino\libraries\M5StickC\src\utility\ST7735_Defines.h
// You can customize the colors in if you want
// http://www.barth-dev.de/online/rgb565-color-picker/
//#define LIGHTGRAY 0xC618      /* 192, 192, 192 */ 
#define LIGHTGRAY 0x7BEF //   128  128  128
#define GRAY 0x4208 //   64  64  64
//#define DARKGRAY   0x0841 //   8   8  8
//#define GREEN  0x0400 //   0 128  0
#define GREEN  0x07E0 //   0 255  0
#define RED    0xF800 // 255   0  0
#define YELLOW 0xFFE0 // 255 255  0
#define ORANGE 0xFD20 // 255 165  0  

// Debuglevel und Fallback für PowerOff
//const int C_Debug_Level = 0; // nix
//const int C_Debug_Level = 1; // Ausgaben
//const int C_Debug_Level = 2; // plus Eingaben
//const int C_Debug_Level = 3; // plus Events
//const int C_Debug_Level = 4; // plus WiFiManager.serialOutput
//const int C_Debug_Level = 5; // plus AtemSwitcher.serialOutput
const int C_Debug_Level = 0;

// Einstellungen
const char* C_Pgm_Name = "ATEM Tally";
const char* C_Pgm_Version = "2021-05-29";
const char* C_AP_SSID = "ATEM-Tally@M5StickC";
const char* C_PubClient_Topic = "Tally/SubClients";
const char* C_SubClient_Topic = "Tally/Inputs";
char chrWiFiSSID[41] = "SSID";
//char chrWiFiSSID[41] = "Technik@Halle";
char chrWiFi_Pwd[41] = "";
char chrHostName[12] = "tl-12-34-56";

// Setup 0
Preferences myPreferences;
unsigned int prefBootCount;
const char* C_Pref_Section = "Tally";
String prefTallyMode;
String prefATEM_Name;
String prefATEM_IPv4;
unsigned int prefATEM_Inputs;
String prefMQTT_Name;
String prefMQTT_IPv4;
unsigned int prefMQTT_Inputs;

unsigned int intSetupCount = 0;
unsigned int intSetupLoop = 0;
unsigned int intSetupPage = 1;
bool isAutoSetup = false;

// Setup 1
int intTimeOutMin; // Timeout in Minuten
int intTimeOutSec; // zusätzlicher Timeout in Sekunden
int intTimeOut;

// Setup 2
const int C_arrTallyMode_Size = 4;
const char* arrTallyModeLong[] = {"", "Tally@ATEM", "Tally@Broker", "Publisher@ATEM"} ;
char arrTallyModeLong_0[16];
char chrTallyModeLong[16];
const char* arrTallyMode[] = {"", "TA", "TB", "PA"} ;
char arrTallyMode_0[3];
char chrTallyMode[3];
int intTallyModeNr;

// Setup 3 und 7
ATEMstd myATEM_Switcher;
// Define the Hostname and/or IP address of your ATEM switcher
const int C_arrATEM_Size = 5;
const char* arrATEM_Name[] = {"", "atem", "atem-tvs-hd", "atem-tvs", "atem-mini"} ;
char arrATEM_Name_0[16];
char chrATEM_Name[16];
const char* arrATEM_IPv4[] = {"0.0.0.0", "192.168.10.240", "192.168.188.82", "192.168.188.81", "192.168.10.240"};
char arrATEM_IPv4_0[16];
char chrATEM_IPv4[16];
unsigned int arrATEM_Inputs[] = {1, 8, 8, 6, 4};
unsigned int intATEM_Inputs;
IPAddress ipATEM;
IPAddress ipATEM_DNS;
unsigned int intATEM_Nr;
unsigned int intATEM_DNS;

// Setup 4 und 8
void callbackMQTT(char* topic, byte* payload, unsigned int length); 
WiFiClient myWiFi_Client;
PubSubClient myMQTT_Client(myWiFi_Client); 
const int C_arrMQTT_Size = 4;
const char* arrMQTT_Name[] = {"", "mqtt-server", "mqtt-server", "mqtt-server"} ;
char arrMQTT_Name_0[16];
char chrMQTT_Name[16];
const char* arrMQTT_IPv4[] = {"0.0.0.0", "192.168.188.70", "192.168.188.70", "0.0.0.0"};
char arrMQTT_IPv4_0[16];
char chrMQTT_IPv4[16];
unsigned int arrMQTT_Inputs[] = {1, 8, 6, 4};
unsigned int intMQTT_Inputs;
IPAddress ipMQTT;
IPAddress ipMQTT_DNS;
unsigned int intMQTT_Nr;
unsigned int intMQTT_DNS;

// Setup 5
WiFiManager myWiFiManager;
WiFiManagerParameter wmp_atem_name("atem_name", "ATEM Switcher");
WiFiManagerParameter wmp_atem_ipv4("atem_ipv4", "ATEM IPv4");
WiFiManagerParameter wmp_mqtt_name("mqtt_name", "MQTT Server");
WiFiManagerParameter wmp_mqtt_ipv4("mqtt_ipv4", "MQTT IPv4");
bool isConfigPortalStarted;
bool isConfigPortalActive;

// Setup 6
WebServer myWebServer(80);

// Loop
bool isAutoOrientation = 0;
unsigned int intOrientation = 0;
unsigned int intOrientationPrevious = 0;
unsigned long intOrientationMillisPrevious = millis();
unsigned long lngButtonAMillis = 0;
unsigned long lngButtonBMillis = 0;
unsigned int intUpdateMillis = 0;

String strLcdSize;
String strLcdText = "1";

unsigned int intPreviewInput;
unsigned int intPreviewInputPrevious = 0;
unsigned int intProgramInput;
unsigned int intProgramInputPrevious = 0;
unsigned int intCameraNumber = 1;
unsigned int intCameraNumberPrevious = 0;
bool isPreviewTallyPrevious = false;
bool isProgramTallyPrevious = false;
bool isLedHatConnected = false;
bool isLedHatConnectedPrevious = false;
bool isLedHatMode = true;

void setup() {
// Serial
  Serial.begin(115200);
// M5
  M5.begin();
  #ifdef M5STICKC
  M5.MPU6886.Init();
  #endif
  #ifdef M5STICKCPLUS
  M5.Imu.Init();
  #endif
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_HAT_RED, OUTPUT);
  pinMode(LED_HAT_GREEN, OUTPUT);
  pinMode(LED_HAT_INPUT, INPUT);
  strLcdSize = String(M5.Lcd.width()) + "*" + String(M5.Lcd.height());
// TALLY Mode Array
  strlcpy(arrTallyModeLong_0, arrTallyModeLong[0], sizeof(arrTallyModeLong_0));
  arrTallyModeLong[0] = arrTallyModeLong_0; // Tricky: Zeiger umbiegen
  strlcpy(arrTallyMode_0, arrTallyMode[0], sizeof(arrTallyMode_0));
  arrTallyMode[0] = arrTallyMode_0; // Tricky: Zeiger umbiegen
// ATEM Array
  strlcpy(arrATEM_Name_0, arrATEM_Name[0], sizeof(arrATEM_Name_0));
  arrATEM_Name[0] = arrATEM_Name_0; // Tricky: Zeiger umbiegen
  strlcpy(arrATEM_IPv4_0, arrATEM_IPv4[0], sizeof(arrATEM_IPv4_0));
  arrATEM_IPv4[0] = arrATEM_IPv4_0;
// MQTT Array
  strlcpy(arrMQTT_Name_0, arrMQTT_Name[0], sizeof(arrMQTT_Name_0));
  arrMQTT_Name[0] = arrMQTT_Name_0; // Tricky: Zeiger umbiegen
  strlcpy(arrMQTT_IPv4_0, arrMQTT_IPv4[0], sizeof(arrMQTT_IPv4_0));
  arrMQTT_IPv4[0] = arrMQTT_IPv4_0;
// Preferences
  myPreferences.begin("ESP32", false); 
  prefBootCount = myPreferences.getUInt("BootCount", 0);
  prefBootCount++;
  myPreferences.putUInt("BootCount", prefBootCount);
  myPreferences.end();
  getPreferences(); 
//  WiFiManager myWiFiManager;
  if (C_Debug_Level < 4) myWiFiManager.setDebugOutput(false); 
  myWiFiManager.addParameter(&wmp_atem_name);
  myWiFiManager.addParameter(&wmp_atem_ipv4);  
  myWiFiManager.addParameter(&wmp_mqtt_name);
  myWiFiManager.addParameter(&wmp_mqtt_ipv4);  
// Timeout
  intTimeOutMin = 9 - C_Debug_Level;
  if (intTimeOutMin < 0) intTimeOutMin = 0;
  intTimeOutSec = 0;
  if (intTimeOutMin == 0) intTimeOutSec = 10;
  intTimeOut = (intTimeOutMin * 60) + intTimeOutSec;
  return;
// GPIO
  pinMode( 0, OUTPUT); // PIN  (INPUT, OUTPUT,       )
  pinMode(26, OUTPUT); // PIN  (INPUT, OUTPUT, ANALOG)
  pinMode(36, INPUT_PULLUP); // PIN  (INPUT,       , ANALOG)
}

void loop() {
  checkLoopEvents();
  if ((intOrientation == intOrientationPrevious) && (intCameraNumber == intCameraNumberPrevious) && (intPreviewInput == intPreviewInputPrevious) && (intProgramInput == intProgramInputPrevious)) return; // nothing changed?
  char chrInputs[3];
  sprintf(chrInputs, "%01d%01d", intPreviewInput, intProgramInput);
  if (C_Debug_Level >= 2) {
    Serial.printf("TALLY Mode: %s %d\n", arrTallyMode[0], isLedHatMode);
    Serial.printf("TALLY Orientation: %d\n", intOrientation);
    Serial.printf("TALLY Camera Number: %d\n", intCameraNumber);
    Serial.printf("ATEM Preview Input: %d\n", intPreviewInput);
    Serial.printf("ATEM Program Input: %d\n", intProgramInput);
  }
  strlcpy(chrTallyMode, arrTallyMode[0], sizeof(chrTallyMode));
  if (strcmp(arrTallyMode_0, "PA") == 0) {
    drawBroker();
    callbackPubClient();
  } else {
    drawTally();
  }
  intOrientationPrevious  = intOrientation;
  intCameraNumberPrevious = intCameraNumber;
  intPreviewInputPrevious = intPreviewInput;
  intProgramInputPrevious = intProgramInput;
}

void checkLoopEvents() {
  // Check for M5Stick Buttons
  checkLoopM5Events();
  // Check for LED Hat
  checkLoopLedHat();
  // Check for Setup 
  if (intSetupPage != 0) checkSetup();
  if (intSetupPage != 0) return;
  // Check for WiFi
  checkLoopWiFi();
  // Check for packets, respond to them etc. Keeping the connection alive!
  checkLoopATEM();
  checkLoopMQTT();
}

void checkLoopM5Events() {
  M5.update();
  if (isAutoOrientation) {
    if (millis() - intOrientationMillisPrevious >= 500 ) {
      setM5Orientation();
      intOrientationMillisPrevious = millis();
    }
  }
  if (M5.BtnA.wasPressed()) {
    if (C_Debug_Level >= 3) Serial.println("M5.BtnA.wasPressed");
    if (arrTallyMode_0[0] == 'T') {
      intCameraNumber++; 
      if (intCameraNumber > intATEM_Inputs) {
        intCameraNumber = 1;
        isLedHatMode = ! isLedHatMode;
        if (C_Debug_Level >= 2) Serial.printf("LED-HAT Mode: %d\n", isLedHatMode);
      }
      if (C_Debug_Level >= 2) Serial.printf("TALLY next Camera Number: %d\n", intCameraNumber);
    } else {
      myATEM_Switcher.doCut();
      if (C_Debug_Level >= 2) Serial.printf("ATEM.doCut\n");
    }
    lngButtonAMillis = millis();
  }
  if (M5.BtnA.isPressed() && lngButtonAMillis != 0 && millis() - lngButtonAMillis >= 500 ) {
    if (C_Debug_Level >= 3) Serial.println("M5.BtnA.isPressed");
    restartESP();
  }
  if (M5.BtnB.wasPressed()) {
    if (C_Debug_Level >= 3) Serial.println("M5.BtnB.wasPressed");
    intOrientation++;
    intOrientation = (intOrientation % 4);
    if (C_Debug_Level >= 2) Serial.printf("M5 new Orientation: %d\n", intOrientation);
//    setM5Orientation();
    lngButtonBMillis = millis();
  }
  if (M5.BtnB.isPressed() && lngButtonBMillis != 0 && millis() - lngButtonBMillis >= 500 ) {
    if (C_Debug_Level >= 3) Serial.println("M5.BtnB.isPressed");
    isAutoOrientation = not isAutoOrientation;
    lngButtonBMillis = 0;
  }
}

void checkLoopLedHat() {
  unsigned int iInput = digitalRead(LED_HAT_INPUT);
  isLedHatConnected = false;
  if (iInput == HIGH) isLedHatConnected = true;
  if (isLedHatConnectedPrevious == isLedHatConnected) return;
  isLedHatConnectedPrevious = isLedHatConnected;
  if (C_Debug_Level >= 1 ) {
    Serial.printf("LED-HAT GPIO %d = %d = ", LED_HAT_INPUT, iInput);
    if (isLedHatConnected == false) Serial.print("dis");
    Serial.println("connected");
  }
}

void checkLoopWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  waitWiFi();
  initM5Lcd();
  intCameraNumberPrevious = 0;
}

void checkLoopATEM() {
  if (arrTallyMode_0[1] != 'A') return;
  myATEM_Switcher.runLoop();
  intPreviewInput = myATEM_Switcher.getPreviewInput();
  if (intPreviewInput > arrATEM_Inputs[0]) intPreviewInput = 9;
  intProgramInput = myATEM_Switcher.getProgramInput();
  if (intProgramInput > arrATEM_Inputs[0]) intProgramInput = 9;
}

void checkLoopMQTT() {
  if (strcmp(arrTallyMode_0, "TA") == 0) return; 
  if (myMQTT_Client.connected() == false) waitMQTT();
  myMQTT_Client.loop();
}

void callbackMQTT(char* chrTopic, byte* payload, unsigned int length) {
  if (C_Debug_Level >= 2) {
    Serial.printf("MQTT %s: ", chrTopic);
    for (int i=0; i<length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.printf("[%d]\n", length);
  }
  if (strcmp(arrTallyMode_0, "PA") == 0) callbackPubClient();
  if (strcmp(arrTallyMode_0, "TB") == 0) callbackSubClient(payload, length);
}

void callbackPubClient() {
  char chrInputs[3];
  sprintf(chrInputs, "%01d%01d", intPreviewInput, intProgramInput);
  myMQTT_Client.publish(C_SubClient_Topic, chrInputs);
}

void callbackSubClient(byte* payload, unsigned int length) {
  int iPayload;
  if (length != 2) {
    flashLED();
    return;
  }
  iPayload = payload[0] - 48;
  if ((iPayload < 1) || (iPayload > 9)) {
    flashLED();
    return;
  }
  iPayload = payload[1] - 48;
  if ((iPayload < 1) || (iPayload > 9)) {
    flashLED();
    return;
  }
  intPreviewInput = payload[0] - 48;
  intProgramInput = payload[1] - 48;
}

void setLED(int iColor) {
  switch (iColor) {
    case RED:
      digitalWrite(LED_PIN, LOW);
      if (isLedHatMode == true) digitalWrite(LED_HAT_RED, HIGH);
      digitalWrite(LED_HAT_GREEN, LOW);
      break;      
    case GREEN:
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(LED_HAT_RED, LOW);
      if (isLedHatMode == true) digitalWrite(LED_HAT_GREEN, HIGH);
      break;      
    case YELLOW:
      digitalWrite(LED_PIN, LOW);
      if (isLedHatMode == true) digitalWrite(LED_HAT_RED, HIGH);
      if (isLedHatMode == true) digitalWrite(LED_HAT_GREEN, HIGH);
      break;      
    default:
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(LED_HAT_RED, LOW);
      digitalWrite(LED_HAT_GREEN, LOW);
      break;      
  }
}

void flashLED() {
  setLED(YELLOW);
  delay(100);
  setLED(BLACK);
}

void stopLED() {
  M5.Lcd.fillScreen(BLACK);
  for (int i = 6; i > 0; i--) {
    flashLED();
    delay(i * 100);
  }
}

void restartESP() {
//  digitalWrite(LED_PIN, LOW);
  setLED(YELLOW);
  stopMQTT();
  stopWiFi();
  if (C_Debug_Level >= 3) Serial.println("M5 Restart");
  Serial.flush();
  stopLED();
  ESP.restart();
}

void stopESP() {
  if (C_Debug_Level >= 1) Serial.println("M5 PowerOff");
  Serial.flush();
  stopMQTT();
  stopWiFi();
  stopLED();
  M5.Axp.PowerOff();
}

void stopWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    delay(500);
  }
  WiFi.mode(WIFI_OFF);
  delay(500);
  if (C_Debug_Level >= 1) {
    Serial.println("WiFi stopped");
  }
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
}

bool waitWiFi() {
/*
typedef enum {
    WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
} wl_status_t;  
*/
  if (C_Debug_Level >= 2) Serial.println("Waiting for WiFi");
  int intLoop = 0;
  int intMillis = 0;
  int iTimeOut = intTimeOut - 1;
  unsigned long lngMillis = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (iTimeOut <= 0) {
      if (C_Debug_Level >= 3) Serial.println("\nM5 TimeOut");
      restartESP();
    }
    iTimeOut = iTimeOut - 1;
    if (C_Debug_Level >= 2) Serial.print(".");
    if (C_Debug_Level >= 3) Serial.print(WiFi.status());
    drawLabel(String(iTimeOut / 60 + 1), YELLOW, BLACK, YELLOW);
    delay(100);
    if ((intSetupPage != 0) || (arrTallyMode_0[0] == 'T')) {
      drawLabel(strLcdText, WHITE, BLACK, BLACK);
    } else {
      drawBroker();
    }
    intMillis = millis();
    lngMillis = millis();
    while (millis() - lngMillis < 1000) {
      if (WiFi.status() == WL_CONNECTED) lngMillis = 0;
      if (M5.BtnA.isPressed() && millis() - intMillis >= 500 ) {
        if (C_Debug_Level >= 3) Serial.println("\nM5.BtnA.isPressed");
        restartESP();
      }
      M5.update();
      if (isConfigPortalActive == true) myWiFiManager.process();
    }
    intLoop++;
    if (intLoop % 60 == 0) {
      if (C_Debug_Level >= 2) Serial.println();
    }
  }
  if (C_Debug_Level >= 2) Serial.println("WiFi connected");
  return true;
}

void stopMQTT() {
  if (strcmp(arrTallyMode_0, "TA") == 0) return;
  if (myMQTT_Client.connected() == false) return;
  if (strcmp(arrTallyMode_0, "PA") == 0) myMQTT_Client.unsubscribe(C_PubClient_Topic);
  if (strcmp(arrTallyMode_0, "TB") == 0) myMQTT_Client.unsubscribe(C_SubClient_Topic);
  delay(1000);
  myMQTT_Client.disconnect();  
  delay(1000);
  if (C_Debug_Level >= 1) {
    Serial.println("MQTT.disconnect done");
  }
}

bool waitMQTT() {
/*
    -4 : MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
    -3 : MQTT_CONNECTION_LOST - the network connection was broken
    -2 : MQTT_CONNECT_FAILED - the network connection failed
    -1 : MQTT_DISCONNECTED - the client is disconnected cleanly
    0 : MQTT_CONNECTED - the client is connected
    1 : MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
    2 : MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
    3 : MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
    4 : MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
    5 : MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
*/
  if (C_Debug_Level >= 2) Serial.println("Waiting for MQTT Broker");
  int intLoop = 0;
  int intMillis = 0;
  int iTimeOut = intTimeOut - 1;
  unsigned long lngMillis = 0;
  myMQTT_Client.connect(chrHostName);
  while (myMQTT_Client.connected() == false) {
    if (iTimeOut <= 0) {
      if (C_Debug_Level >= 3) Serial.println("\nM5 TimeOut");
      restartESP();
    }
    iTimeOut = iTimeOut - 1;
    if (C_Debug_Level >= 2) Serial.print(".");
    if (C_Debug_Level >= 3) Serial.print(myMQTT_Client.state());
    drawLabel(String(iTimeOut / 60 + 1), YELLOW, BLACK, YELLOW);
    delay(100);
    if ((intSetupPage != 0) || (arrTallyMode_0[0] == 'T')) {
      drawLabel(strLcdText, WHITE, BLACK, BLACK);
    } else {
      drawBroker();
    }
    intMillis = millis();
    lngMillis = millis();
    while (millis() - lngMillis < 1000) {
      if (myMQTT_Client.connected()) lngMillis = 0;
      if (M5.BtnA.isPressed() && millis() - intMillis >= 500 ) {
        if (C_Debug_Level >= 3) Serial.println("\nM5.BtnA.isPressed");
        restartESP();
      }
      M5.update();
    }
    intLoop++;
    if (intLoop % 60 == 0) {
      if (C_Debug_Level >= 2) Serial.println();
    }
    if (intLoop % 10 == 0) {
      myMQTT_Client.connect(chrHostName);
    }
  }
  if (C_Debug_Level >= 2) Serial.println("MQTT connected");
  if (strcmp(arrTallyMode_0, "PA") == 0) {
    myMQTT_Client.subscribe(C_PubClient_Topic);
    if (C_Debug_Level >= 1) {
      Serial.printf("MQTT.subscribe %s\n", C_PubClient_Topic);
    }
  }
  if (strcmp(arrTallyMode_0, "TB") == 0) {
    myMQTT_Client.subscribe(C_SubClient_Topic);
    if (C_Debug_Level >= 1) {
      Serial.printf("MQTT.subscribe %s\n", C_SubClient_Topic);
    }
  }
  return true;  
}

void checkSetup() {
  intSetupCount++;
  if (C_Debug_Level >= 2) {
    Serial.printf("\nStarting Setup Mode %d at Count %d\n", intSetupPage, intSetupCount);
  }
  initM5Lcd();
  intSetupLoop = 0;
  while (intSetupPage == 1) {
    getTimeOut();
    waitButton(intSetupPage);
    isAutoSetup = false;
    if (intSetupPage < 1) {
      intSetupPage = 2;
      isAutoSetup = true;
    }
    intSetupLoop++;
  }
  intSetupLoop = 0;
  while (intSetupPage == 2) {
    getTallyMode();
    waitButton(intSetupPage);
    if (intSetupPage < 2) {
      clearPreferences();
      return;
    }
    if (intSetupPage > 2) setTallyMode();
    intSetupLoop++;
  }
  intSetupLoop = 0;
  while (intSetupPage == 3) {
    if (arrTallyMode_0[1] == 'A') {
      getATEM_Setup();
      waitButton(intSetupPage);
      if (intSetupPage < 3) return;
      if (intSetupPage > 3) setATEM_Setup();
      intSetupLoop++;
    } else {
      intSetupPage++;
    }
  }
  intSetupLoop = 0;
  while (intSetupPage == 4) {
    if ((arrTallyMode_0[0] == 'P') || (arrTallyMode_0[1] == 'B')) {
      getMQTT_Setup();
      waitButton(intSetupPage);
      if (intSetupPage < 4) return;
      if (intSetupPage > 4) setMQTT_Setup();
      intSetupLoop++;
    } else {
      intSetupPage++;
    }
  }
  while (intSetupPage == 5) {
    startWiFiManager();
    waitButton(intSetupPage);
    if (intSetupPage < 5) {
      if (C_Debug_Level >= 1) Serial.println("WM.resetSettings");
      myWiFiManager.resetSettings();
      return;
    }
  }
  startWiFi();
  while (intSetupPage == 6) {
    startWebServer();
    waitButton(intSetupPage);
    stopWebServer();
    if (intSetupPage < 6) return;
    if (intSetupPage == 6) delay(1000);
  }
  intATEM_DNS = 0;
  while (intSetupPage == 7) {
    if (arrTallyMode_0[1] == 'A') {
      getATEM_DNS();
      waitButton(intSetupPage);
      if (intSetupPage < 7) return;
    } else {
      intSetupPage++;
    }
  }
  intMQTT_DNS = 0;
  while (intSetupPage == 8) {
    if ((arrTallyMode_0[0] == 'P') || (arrTallyMode_0[1] == 'B')) {
      getMQTT_DNS();
      waitButton(intSetupPage);
      if (intSetupPage < 8) return;
    } else {
      intSetupPage++;
    }
  }
  if (intSetupPage == 9) {
    startServerCommunication();
    isAutoSetup = true;
    if ((arrTallyMode_0[0] == 'P') || (arrTallyMode_0[1] == 'B')) waitButton(intSetupPage);
    if (intSetupPage < 9) return;
  }
  savePreferences();
  intSetupPage = 0;
}

void waitButton(int i1) {
  int iTimeOut = intTimeOut;
  int intLedState = HIGH;
  unsigned long lngIntervall = 0;
  unsigned long lngMillis = 0;
//  digitalWrite(LED_PIN, LOW);
  if (C_Debug_Level >= 2) Serial.printf("M5 waiting for BtnA at Setup Mode %d\n", intSetupPage);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.println();
  switch (i1) {
    case 0:
      M5.Lcd.print(" Reboot mit ");
      break;
    case 5:
      if (isConfigPortalActive) {
        M5.Lcd.print(" auf Eingaben und/oder ");
      } else {
        M5.Lcd.print(" Weiter mit ");
      }
      break;
    case 6:
      M5.Lcd.print(" HTTP Update oder ");
      break;
    case 9:
      M5.Lcd.print(" Warten ohne ");
      break;
    default:
      M5.Lcd.print(" Auswahl oder ");
      break;
  }
  if (isAutoSetup == false) M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.println("M5");
  M5.Lcd.setTextColor(WHITE);
  while (M5.BtnA.wasPressed() == false) {
    if (millis() - lngMillis >= lngIntervall) {
      lngMillis = millis();
      if (intLedState == HIGH) {
        intLedState = LOW;
        setLED(RED);
//        digitalWrite(LED_PIN, intLedState);
        lngIntervall = 3000;
      } else {
        intLedState = HIGH;     
        setLED(GREEN);
        lngIntervall = 3000;
        iTimeOut = iTimeOut - 6;
        if (isAutoSetup == true) {
          intSetupPage++;
          return;
        }
      }
      if (iTimeOut <= 0) {
        if (C_Debug_Level >= 1) Serial.println("M5 Timeout");
        intSetupPage--;
        if (intSetupPage <= 0) stopESP();
        restartESP();
      }
    }
    M5.update();
    if (checkBtnB(i1) == true) return;
    if (isConfigPortalActive == true) myWiFiManager.process(); 
    if (i1 >= 6) myWebServer.handleClient(); 
    if ((i1 >= 7) && (arrTallyMode_0[1] == 'A')) myATEM_Switcher.runLoop();
  }
  if (C_Debug_Level >= 3) Serial.println("M5.BtnA.wasPressed");
  setLED(BLACK);
//  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.fillScreen(BLACK);
  lngMillis = millis();
  while (M5.BtnA.isPressed() == true) {
    if (millis() - lngMillis >= 500) {
      if (C_Debug_Level >= 3) Serial.println("M5.BtnA.isPressed");
      intSetupPage--;
      return;
    }
    M5.update();
  }
  lngButtonAMillis = 0;
  intSetupPage++;
  return;
}

bool checkBtnB(int i1) {
  if (M5.BtnB.wasPressed() == false) return false;  
  if (C_Debug_Level >= 3) Serial.println("M5.BtnB.wasPressed");
  switch (i1) {
    case 1:
      return true;
    case 2:
      return true;
    case 3:
      return true;
    case 4:
      return true;
    case 7:
      return true;
    case 8:
      return true;
    default:
      return false;
  }
}

void initM5Lcd() {
  // Initialize Display
  if (C_Debug_Level >= 2) Serial.println("M5 initializing Display");
  // Lcd display test
  setLED(BLACK);
//  digitalWrite(LED_PIN, LOW);
  M5.Lcd.fillScreen(BLACK);
  delay(1000);
//  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.fillScreen(WHITE);
  delay(1000);
  setLED(RED);
//  digitalWrite(LED_PIN, LOW);
  M5.Lcd.fillScreen(RED);
  delay(1000);
  setLED(GREEN);
//  digitalWrite(LED_PIN, LOW);
  M5.Lcd.fillScreen(GREEN);
  delay(1000);
  setLED(YELLOW);
//  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.fillScreen(YELLOW);
  delay(1000);
  setLED(BLACK);
//  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setRotation(1); //drehen des Displays
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
}

void clearM5Lcd() {
  setLED(BLACK);
//  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.fillScreen(BLACK);
  delay(100);
  M5.Lcd.setCursor(0, 0);
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(); 
  #endif
  M5.Lcd.setTextColor(WHITE);
  if (isAutoSetup == true) M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.printf(" %d\.%d", intSetupCount, intSetupPage);
}

// Setup 1
void getTimeOut() {
  if (intSetupLoop == 0) stopWiFi;
  if (intSetupLoop == 0) printM5Info1();
  if (intSetupLoop != 0) {
    intTimeOutSec = 0;
    intTimeOutMin++;
    if (intTimeOutMin > 9) intTimeOutMin = 0;
    if (intTimeOutMin == 0) intTimeOutSec = 10;
    intTimeOut = (intTimeOutMin * 60) + intTimeOutSec;
  }
  printM5Info2();
}

void printM5Info1() {
  if (C_Debug_Level >= 2) {
    Serial.printf("M5 LCD Size %s\n", strLcdSize);
    Serial.printf("M5 Akku %.3fV\n", M5.Axp.GetBatVoltage());  
    Serial.printf("WiFi Status %d\n", WiFi.status());  
  }
}

void printM5Info2() {
  if (C_Debug_Level >= 2) {
    Serial.printf("TALLY TimeOut: %d:%02d\n", intTimeOutMin, intTimeOutSec);
  }
  clearM5Lcd();
  M5.Lcd.printf(" %s %s\n\n", C_Pgm_Name, C_Pgm_Version);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.printf(" Boot Nr. %d", prefBootCount);  
  M5.Lcd.print(" Akku ");  
  double dblBat = M5.Axp.GetBatVoltage(); 
  if (dblBat < 3.5) M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.printf("%sV\n\n", String(dblBat));  
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print(" Zeit ");
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.printf("%d:%02d", intTimeOutMin, intTimeOutSec);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print(" bis TimeOut\n\n");
  #ifdef M5STICKCPLUS 
    M5.Lcd.print(" WiFi ");
    if (WiFi.status() != WL_NO_SHIELD) {
      M5.Lcd.print("eingeschaltet\n");
    } else {
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.print("ausgeschaltet\n");
    }
    M5.Lcd.println(); 
  #endif
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.print("   M5"); 
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print(" kurz OK lang Setup\n"); 
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(); 
  #endif
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.print(" BtnB"); 
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print(" Auswahl/Ausrichtung\n");
}

// Setup 2
void getTallyMode() {
  unsigned int i;
  if (intSetupLoop == 0) {
    intTallyModeNr = 0;
    prefTallyMode.toCharArray(arrTallyMode_0, sizeof(arrTallyMode_0));
    for (i = 1; i < C_arrTallyMode_Size; i++) {
      if (strcmp(arrTallyMode_0, arrTallyMode[i]) == 0) {
        strlcpy(arrTallyModeLong_0, arrTallyModeLong[i], sizeof(arrTallyModeLong_0));
      }
    }
  }
  if (intSetupLoop != 0) {
    intTallyModeNr++;
    if (intTallyModeNr >= C_arrTallyMode_Size) intTallyModeNr = 0;
  }
  printTallyModeInfo1();
  for (i = 0; i < C_arrTallyMode_Size; i++) {
    if (printTallyModeInfo2(i) == false) {
      if (intTallyModeNr == 0) intTallyModeNr++;
    }
  }
}

void setTallyMode() {
  if (intTallyModeNr != 0) {
    strlcpy(arrTallyMode_0, arrTallyMode[intTallyModeNr], sizeof(arrTallyMode_0));
  }
  String strHostName = WiFi.macAddress();
  strHostName.replace(":", "-");
  strHostName = String(arrTallyMode_0) + strHostName.substring(8);
  strHostName.toLowerCase();
  strHostName.toCharArray(chrHostName, 12);
  if (arrTallyMode_0[0] == 'T') {
    intOrientation = 0;
  } else {
    intOrientation = 1;
  }
  if (C_Debug_Level >= 1) {
    Serial.printf("Tally Mode Nr. %d %s selected\n", intTallyModeNr, arrTallyMode_0);
  }  
}

void printTallyModeInfo1() {
  strlcpy(chrTallyMode, arrTallyMode[intTallyModeNr], sizeof(chrTallyMode));
  if (C_Debug_Level >= 2) Serial.printf("TALLY Mode new Number: %d %s\n", intTallyModeNr, chrTallyMode);
  clearM5Lcd();
  M5.Lcd.print(" Modus Wahl mit ");
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.print("BtnB\n\n");
  M5.Lcd.setTextColor(WHITE);
}

bool printTallyModeInfo2(int i1) {
  strlcpy(chrTallyModeLong, arrTallyModeLong[i1], sizeof(chrTallyModeLong));
  strlcpy(chrTallyMode, arrTallyMode[i1], sizeof(chrTallyMode));
  unsigned int iLen = strlen(chrTallyMode);
  if ((C_Debug_Level >= 1) && (intSetupLoop == 0)) {
    Serial.printf("TALLY Mode Nr. %d %s %s \n", i1, chrTallyMode, chrTallyModeLong);
  }
  M5.Lcd.setTextColor(WHITE);
//  if (i1 >= 2) M5.Lcd.setTextColor(RED);
  if (iLen == 0) {
    return false;
  }
  if (i1 == intTallyModeNr) M5.Lcd.setTextColor(GREEN);
  M5.Lcd.printf(" %s %s \n", chrTallyMode, chrTallyModeLong);
  if (i1 == 0) M5.Lcd.println();
  #ifdef M5STICKCPLUS 
    M5.Lcd.println();
  #endif
  return true;
}

// Setup 3
void getATEM_Setup() {
  if (intSetupLoop == 0) {
    intATEM_Nr = 0;
  }
  if (intSetupLoop != 0) {
    intATEM_Nr++;
    if (intATEM_Nr >= C_arrATEM_Size) intATEM_Nr = 0;
  }
  printATEM_SetupInfo1();
  for (int i = 0; i < C_arrATEM_Size; i++) {
    if (printATEM_SetupInfo2(i) == false) {
      if (intATEM_Nr == 0) intATEM_Nr++;
    }
  }
}

void setATEM_Setup() {
  if (arrTallyMode_0[1] != 'A') return;
  if (intATEM_Nr != 0) {
    strlcpy(arrATEM_Name_0, arrATEM_Name[intATEM_Nr], sizeof(arrATEM_Name_0));
    strlcpy(arrATEM_IPv4_0, arrATEM_IPv4[intATEM_Nr], sizeof(arrATEM_IPv4_0));
    intATEM_Inputs = arrATEM_Inputs[intATEM_Nr];
    arrATEM_Inputs[0] = intATEM_Inputs;
  }
  if (C_Debug_Level >= 1) {
    Serial.printf("ATEM Nr. %d %s ", intATEM_Nr, arrATEM_Name_0);
    Serial.printf("on %s with %d Inputs selected\n", arrATEM_IPv4_0, intATEM_Inputs);
  }
}

void printATEM_SetupInfo1() {
  if (C_Debug_Level >= 2) Serial.printf("ATEM new Number: %d\n", intATEM_Nr);
  clearM5Lcd();
  M5.Lcd.print(" ATEM Wahl mit ");
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.print("BtnB\n\n");
  M5.Lcd.setTextColor(WHITE);
}

bool printATEM_SetupInfo2(int i1) {
//  String sText = arrATEM_Name[i1];
//  int iLength = sText.length();
  strlcpy(chrATEM_Name, arrATEM_Name[i1], sizeof(chrATEM_Name));
  strlcpy(chrATEM_IPv4, arrATEM_IPv4[i1], sizeof(chrATEM_IPv4));
  intATEM_Inputs = arrATEM_Inputs[i1];
  unsigned int iLen = strlen(chrATEM_Name);
  if ((C_Debug_Level >= 1) && (intSetupLoop == 0)) {
    Serial.printf("ATEM Nr. %d %s[%d] ", i1, chrATEM_Name, iLen);
    Serial.printf("on %s with %d Inputs\n", chrATEM_IPv4, intATEM_Inputs);
  }
  M5.Lcd.setTextColor(WHITE);
  if (iLen == 0) {
    return false;
  }
  if (i1 == intATEM_Nr) M5.Lcd.setTextColor(GREEN);
  M5.Lcd.printf(" %d %s ", intATEM_Inputs, chrATEM_Name);
  #ifdef M5STICKCPLUS 
    M5.Lcd.print(chrATEM_IPv4);
  #endif
  M5.Lcd.println();
  if (i1 == 0) M5.Lcd.println();
  #ifdef M5STICKCPLUS 
    M5.Lcd.println();
  #endif
  return true;
}

// Setup 4
void getMQTT_Setup() {
  if (intSetupLoop == 0) {
    intMQTT_Nr = 0;
  }
  if (intSetupLoop != 0) {
    intMQTT_Nr++;
    if (intMQTT_Nr >= C_arrMQTT_Size) intMQTT_Nr = 0;
  }
  printMQTT_SetupInfo1();
  for (int i = 0; i < C_arrMQTT_Size; i++) {
    if (printMQTT_SetupInfo2(i) == false) {
      if (intMQTT_Nr == 0) intMQTT_Nr++;
    }
  }
}

void setMQTT_Setup() {
  if (strcmp(arrTallyMode_0, "TA") == 0) return;
  if (intMQTT_Nr != 0) {
    strlcpy(arrMQTT_Name_0, arrMQTT_Name[intMQTT_Nr], sizeof(arrMQTT_Name_0));
    strlcpy(arrMQTT_IPv4_0, arrMQTT_IPv4[intMQTT_Nr], sizeof(arrMQTT_IPv4_0));
    intMQTT_Inputs = arrMQTT_Inputs[intMQTT_Nr];
    arrMQTT_Inputs[0] = intMQTT_Inputs;
  }
  if (C_Debug_Level >= 1) {
    Serial.printf("MQTT Nr. %d %s ", intMQTT_Nr, arrMQTT_Name_0);
    Serial.printf("on %s with %d Inputs selected\n", arrMQTT_IPv4_0, intMQTT_Inputs);
  }
}

void printMQTT_SetupInfo1() {
  if (C_Debug_Level >= 2) Serial.printf("MQTT new Number: %d\n", intMQTT_Nr);
  clearM5Lcd();
  M5.Lcd.print(" Broker Wahl mit ");
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.print("BtnB\n\n");
  M5.Lcd.setTextColor(WHITE);
}

bool printMQTT_SetupInfo2(int i1) {
  strlcpy(chrMQTT_Name, arrMQTT_Name[i1], sizeof(chrMQTT_Name));
  strlcpy(chrMQTT_IPv4, arrMQTT_IPv4[i1], sizeof(chrMQTT_IPv4));
  intMQTT_Inputs = arrMQTT_Inputs[i1];
  unsigned int iLen = strlen(chrMQTT_Name);
  if ((C_Debug_Level >= 1) && (intSetupLoop == 0)) {
    Serial.printf("MQTT Nr. %d %s[%d] ", i1, chrMQTT_Name, iLen);
    Serial.printf("on %s with %d Inputs\n", chrMQTT_IPv4, intMQTT_Inputs);
  }
  M5.Lcd.setTextColor(WHITE);
  if (iLen == 0) {
    return false;
  }
  if (i1 == intMQTT_Nr) M5.Lcd.setTextColor(GREEN);
  M5.Lcd.printf(" %d %s ", intMQTT_Inputs, chrMQTT_Name);
  #ifdef M5STICKCPLUS 
    M5.Lcd.print(chrMQTT_IPv4);
  #endif
  M5.Lcd.println();
  if (i1 == 0) M5.Lcd.println();
  #ifdef M5STICKCPLUS 
    M5.Lcd.println();
  #endif
  return true;
}

// Setup 5
void startWiFiManager() {
  if ((intSetupPage > 5) && (WiFi.status() == WL_CONNECTED)) return;
  myWiFiManager.setAPStaticIPConfig(IPAddress(192,168,5,1), IPAddress(192,168,5,1), IPAddress(255,255,255,0));
  myWiFiManager.setHostname(chrHostName);
  myWiFiManager.setConfigPortalBlocking(false);
  myWiFiManager.setConfigPortalTimeout(intTimeOut);
  myWiFiManager.setAPCallback(wm_configModeCallback);
  myWiFiManager.setSaveConfigCallback(wm_saveConfigCallback);
  printManagerInfo1();
// id/name, placeholder/prompt, default, length
  wmp_atem_name.setValue(arrATEM_Name_0, (sizeof(arrATEM_Name_0) - 1));
  wmp_atem_ipv4.setValue(arrATEM_IPv4_0, (sizeof(arrATEM_IPv4_0) - 1));
  wmp_mqtt_name.setValue(arrMQTT_Name_0, (sizeof(arrMQTT_Name_0) - 1));
  wmp_mqtt_ipv4.setValue(arrMQTT_IPv4_0, (sizeof(arrMQTT_IPv4_0) - 1));
//  myWiFiManager.autoConnect(C_AP_SSID);
//  myWiFiManager.startConfigPortal(C_AP_SSID);
  if (intSetupCount == 1) {
    isConfigPortalStarted = false;
    isConfigPortalActive = false;
    myWiFiManager.autoConnect(C_AP_SSID);
  } else {
    isConfigPortalStarted = true;
    isConfigPortalActive = false;
    myWiFiManager.startConfigPortal(C_AP_SSID);
  }
  waitWiFi();
  if (isConfigPortalStarted == true) {
    waitButton(intSetupPage);
    if (isConfigPortalActive == true) myWiFiManager.stopConfigPortal();
    delay(1000);
  }
  isConfigPortalActive = false;
  printManagerInfo2();
}

void printManagerInfo1() {
  if (C_Debug_Level >= 2) {
    Serial.print("WiFi Settings \n");
    Serial.print("WiFi MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.print("WiFi Hostname: ");
    Serial.println(chrHostName);  clearM5Lcd();
    Serial.println("WM starting with");
    Serial.printf("WM TimeOut %d:%02d\n", intTimeOutMin, intTimeOutSec);
    Serial.printf("WM ATEM Name %s\n", arrATEM_Name_0);
    Serial.printf("WM ATEM IPv4 %s\n", arrATEM_IPv4_0);
    Serial.printf("WM MQTT Name %s\n", arrMQTT_Name_0);
    Serial.printf("WM MQTT IPv4 %s\n", arrMQTT_IPv4_0);
  }
  clearM5Lcd();
  M5.Lcd.print(" WiFi Manager\n\n");
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.printf(" TimeOut %d:%02d Minute(n)\n", intTimeOutMin, intTimeOutSec);
}

void wm_configModeCallback (WiFiManager *myWiFiManager) {
  isConfigPortalActive = true;
  Serial.println("WM Entered config mode on");
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.println(WiFi.softAPIP());
  clearM5Lcd();
  M5.Lcd.print(" WiFi Manager\n\n");
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.print(" wartet unter\n\n");
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print(" SSID ");
  M5.Lcd.println(myWiFiManager->getConfigPortalSSID());
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(); 
  #endif
  M5.Lcd.print(" IPv4 ");
  M5.Lcd.println(WiFi.softAPIP());
}

//callback notifying us of the need to save config
void wm_saveConfigCallback () {
  isConfigPortalActive = false;
//  wmSaveConfig = true;
  Serial.print("\nWM saveconfigCallback\n");
  Serial.printf("WM %s=%s\n", wmp_atem_name.getID(), wmp_atem_name.getValue());
  Serial.printf("WM %s=%s\n", wmp_atem_ipv4.getID(), wmp_atem_ipv4.getValue());
  Serial.printf("WM %s=%s\n", wmp_mqtt_name.getID(), wmp_mqtt_name.getValue());
  Serial.printf("WM %s=%s\n", wmp_mqtt_ipv4.getID(), wmp_mqtt_ipv4.getValue());
  strcpy(arrATEM_Name_0, wmp_atem_name.getValue());
  strcpy(arrATEM_IPv4_0, wmp_atem_ipv4.getValue());
  strcpy(arrMQTT_Name_0, wmp_mqtt_name.getValue());
  strcpy(arrMQTT_IPv4_0, wmp_mqtt_ipv4.getValue());
  Serial.print("WM saveconfigCallback done\n");
}

void printManagerInfo2() {
  if (C_Debug_Level >= 1) {
    Serial.println("WiFiManager ready");
  }
  WiFi.SSID().toCharArray(chrWiFiSSID, sizeof(chrWiFiSSID));
  if (C_Debug_Level >= 2) {
    Serial.print("WiFi SSID ");
    Serial.println(chrWiFiSSID);
    Serial.print("ATEM Name ");
    Serial.println(arrATEM_Name_0);
    Serial.print("ATEM IPv4 ");
    Serial.println(arrATEM_IPv4_0);
    Serial.print("MQTT Name ");
    Serial.println(arrMQTT_Name_0);
    Serial.print("MQTT IPv4 ");
    Serial.println(arrMQTT_IPv4_0);
  }
  clearM5Lcd();
  M5.Lcd.print(" WiFi Manager\n\n");
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print(" SSID ");
  M5.Lcd.println(chrWiFiSSID);
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(); 
  #endif
  M5.Lcd.print(" IPv4 ");
  M5.Lcd.println(WiFi.localIP());
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(); 
  #endif
  M5.Lcd.printf(" ATEM %s\n", arrATEM_Name_0);
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(); 
  #endif
  M5.Lcd.print(" IPv4 ");
  if (!ipATEM.fromString(arrATEM_IPv4_0)) M5.Lcd.setTextColor(RED);
  M5.Lcd.println(arrATEM_IPv4_0);
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(); 
  #endif
  M5.Lcd.printf(" MQTT %s\n", arrMQTT_Name_0);
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(); 
  #endif
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print(" IPv4 ");
  if (!ipMQTT.fromString(arrMQTT_IPv4_0)) M5.Lcd.setTextColor(RED);
  M5.Lcd.println(arrMQTT_IPv4_0);
}

// Setup 6
void startWiFi() {
  if ((intSetupPage > 6) && (WiFi.status() == WL_CONNECTED)) return;
  if (intSetupPage > 6) intSetupPage = 6;
  printWiFiInfo1();
  waitWiFi();
  printWiFiInfo2();
}

void printWiFiInfo1() {
  // Connecting WiFi
  if (C_Debug_Level >= 3) {
    Serial.printf("WiFi.status=%d\n", WiFi.status());
  }
  if (C_Debug_Level >= 2) {
//    Serial.printf("WiFi Name: %s\n", chrHostName);
    Serial.printf("WiFi connecting to %s\n", chrWiFiSSID);
  }
  if (intSetupPage != 6) return;
  clearM5Lcd();
  M5.Lcd.print(" WiFi Einstellungen\n\n");
  M5.Lcd.setTextColor(WHITE);
  // MAC address
  M5.Lcd.print("  MAC ");
  M5.Lcd.print(WiFi.macAddress());
  M5.Lcd.println(); 
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(); 
  #endif
  M5.Lcd.printf(" NAME %s\n\n", chrHostName);
  M5.Lcd.print(" SSID ");
  M5.Lcd.print(chrWiFiSSID);
  M5.Lcd.println();
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(); 
  #endif
  M5.update();
}

void printWiFiInfo2() {
  IPAddress ip = WiFi.localIP(); 
  if (C_Debug_Level >= 2) {
    Serial.println();  
  }
  if (C_Debug_Level >= 1) {
    Serial.print("WiFi connected to ");
    Serial.println(chrWiFiSSID);
    Serial.print("WiFi IPv4: ");
    Serial.println(ip);  
    Serial.print("WiFi Signal strength: ");
    Serial.println(WiFi.RSSI());  
  }
  if (intSetupPage != 6) return;
  M5.Lcd.print(" IPv4 ");
  M5.Lcd.println(ip);
  M5.update();
}

// Setup 7
void getATEM_DNS() {
  // Connecting AtemSwitcher
  int rcDNS;
  if (intATEM_DNS == 0) {
    rcDNS = WiFi.hostByName(arrATEM_Name_0, ipATEM_DNS);
    printATEM_DNSInfo1(rcDNS);
    intATEM_DNS++;
  }
  intATEM_DNS++;
  if (intATEM_DNS >= 3) intATEM_DNS = 1;
  if (intATEM_DNS == 1) {
    rcDNS = WiFi.hostByName(arrATEM_Name_0, ipATEM_DNS);
    ipATEM = ipATEM_DNS;
  } else {
    ipATEM.fromString(arrATEM_IPv4_0);
  }
  printATEM_DNSInfo2(intATEM_DNS, rcDNS);
}

void printATEM_DNSInfo1(int i1) {
  if (C_Debug_Level >= 1) {
    Serial.print("ATEM DNS: ");
    Serial.println(i1); 
    Serial.print("ATEM DNS IPv4: ");
    Serial.println(ipATEM_DNS);
  }
}

void printATEM_DNSInfo2(int i1, int i2) {
  if (C_Debug_Level >= 2) {
    Serial.print("ATEM DNS IPv4: ");
    Serial.print(ipATEM);
    Serial.println(" selected");
  }
  clearM5Lcd();
  M5.Lcd.print(" ATEM Einstellungen\n\n");
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.printf(" NAME %s\n\n", arrATEM_Name_0);
  M5.Lcd.print("  DNS ");
  if (i1 == 1) M5.Lcd.setTextColor(GREEN);
  if ((i1 == 1) && (i2 != 1)) M5.Lcd.setTextColor(RED);
  M5.Lcd.println(ipATEM_DNS);
  M5.Lcd.println();
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print(" oder ");
  if (i1 != 1) M5.Lcd.setTextColor(GREEN);
  M5.Lcd.println(arrATEM_IPv4_0);
  M5.Lcd.setTextColor(WHITE);
}

// Setup 8
void getMQTT_DNS() {
  // Connecting MQTT Server (Broker)
  int rcDNS;
  rcDNS = WiFi.hostByName(arrMQTT_Name_0, ipMQTT_DNS);
  if (intMQTT_DNS == 0) {
    intMQTT_DNS++;
    printMQTT_DNSInfo1(rcDNS);
  }
  intMQTT_DNS++;
  if (intMQTT_DNS >= 3) intMQTT_DNS = 1;
  if (intMQTT_DNS == 1) {
    rcDNS = WiFi.hostByName(arrMQTT_Name_0, ipMQTT_DNS);
    ipMQTT = ipMQTT_DNS;
  } else {
    ipMQTT.fromString(arrMQTT_IPv4_0);
  }
  printMQTT_DNSInfo2(intMQTT_DNS, rcDNS);
}

void printMQTT_DNSInfo1(int i1) {
  if (C_Debug_Level >= 1) {
    Serial.print("MQTT DNS: ");
    Serial.println(i1); 
    Serial.print("MQTT DNS IPv4: ");
    Serial.println(ipMQTT_DNS);
  }
}

void printMQTT_DNSInfo2(int i1, int i2) {
  if (C_Debug_Level >= 2) {
    Serial.print("MQTT DNS IPv4: ");
    Serial.print(ipMQTT);
    Serial.println(" selected");
  }
  clearM5Lcd();
  M5.Lcd.print(" Broker Einstellungen\n\n");
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.printf(" NAME %s\n\n", arrMQTT_Name_0);
  M5.Lcd.print("  DNS ");
  if (i1 == 1) M5.Lcd.setTextColor(GREEN);
  if ((i1 == 1) && (i2 != 1)) M5.Lcd.setTextColor(RED);
  M5.Lcd.println(ipMQTT_DNS);
  M5.Lcd.println();
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print(" oder ");
  if (i1 != 1) M5.Lcd.setTextColor(GREEN);
  M5.Lcd.println(arrMQTT_IPv4_0);
  M5.Lcd.setTextColor(WHITE);
}

// Setup 9
void startServerCommunication() {
  intCameraNumberPrevious = 0;
  intCameraNumber = 1;
  if (arrTallyMode_0[1] == 'A') startATEM();
  if (strcmp(arrTallyMode_0, "TA") != 0) startMQTT();
}

void startATEM() {
  String sText = ipATEM.toString();
  sText.toCharArray(arrATEM_IPv4_0, sizeof(arrATEM_IPv4_0));
  myATEM_Switcher.begin(ipATEM);
  if (C_Debug_Level >= 5) myATEM_Switcher.serialOutput(0x80);
  myATEM_Switcher.connect();
  intATEM_Inputs = arrATEM_Inputs[0];
  if (C_Debug_Level >= 1) {
    Serial.printf("ATEM.begin done with %d Inputs\n", intATEM_Inputs);
    Serial.println("ATEM last Camera Number: 0");
    Serial.println("ATEM next Camera Number: 1");
  }
}

void startMQTT() {
  String sText = ipMQTT.toString();
  sText.toCharArray(arrMQTT_IPv4_0, sizeof(arrMQTT_IPv4_0));
  printMQTTInfo1();
  myMQTT_Client.setCallback(callbackMQTT);
  myMQTT_Client.setKeepAlive(60);
  myMQTT_Client.setServer(ipMQTT, 1883);
  waitMQTT();
  printMQTTInfo2();
  if (strcmp(arrTallyMode_0, "TB") != 0) return;
  myMQTT_Client.publish(C_PubClient_Topic,  chrHostName);
}

void printMQTTInfo1() {
  String sText = ipMQTT.toString();
  sText.toCharArray(chrMQTT_IPv4, sizeof(chrMQTT_IPv4));
  if (C_Debug_Level >= 3) {
    Serial.printf("MQTT.connect to %s as %s\n", chrMQTT_IPv4, chrHostName);
  }
  if (intSetupPage != 9) return;
  clearM5Lcd();
  M5.Lcd.print(" Broker Start\n\n");  
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.printf(" IPv4 %s\n", chrMQTT_IPv4);  
}

void printMQTTInfo2() {
}

// Setup Ende
void clearPreferences() {
  if (C_Debug_Level >= 1) Serial.println("Preferences.clear");
  myPreferences.begin(C_Pref_Section, false); 
  myPreferences.clear();  
  prefTallyMode = "TA";
  prefATEM_Name = "";
  prefATEM_IPv4 = "0.0.0.0";
  prefATEM_Inputs = 1;
  prefMQTT_Name = "";
  prefMQTT_IPv4 = "0.0.0.0";
  prefMQTT_Inputs = 1;
  myPreferences.end(); 
}

void getPreferences() {
  myPreferences.begin(C_Pref_Section, false); 
// Tally Mode
  strlcpy(chrTallyMode, arrTallyMode[0], sizeof(chrTallyMode));
  prefTallyMode = myPreferences.getString("TallyMode", chrTallyMode);
// ATEM Name
  strlcpy(chrATEM_Name, arrATEM_Name[0], sizeof(chrATEM_Name));
  prefATEM_Name = myPreferences.getString("AtemName", chrATEM_Name);
  prefATEM_Name.toCharArray(arrATEM_Name_0, sizeof(arrATEM_Name_0));
// ATEM IPv4
  strlcpy(chrATEM_IPv4, arrATEM_IPv4[0], sizeof(chrATEM_IPv4));
  prefATEM_IPv4 = myPreferences.getString("AtemIPv4", chrATEM_IPv4);
  prefATEM_IPv4.toCharArray(arrATEM_IPv4_0, sizeof(arrATEM_IPv4_0));
// ATEM Inputs
  intATEM_Inputs = arrATEM_Inputs[0];
  prefATEM_Inputs = myPreferences.getUInt("AtemInputs", intATEM_Inputs);
  arrATEM_Inputs[0] = prefATEM_Inputs;
// MQTT Name
  strlcpy(chrMQTT_Name, arrMQTT_Name[0], sizeof(chrMQTT_Name));
  prefMQTT_Name = myPreferences.getString("MqttName", chrMQTT_Name);
  prefMQTT_Name.toCharArray(arrMQTT_Name_0, sizeof(arrMQTT_Name_0));
// MQTT IPv4
  strlcpy(chrMQTT_IPv4, arrMQTT_IPv4[0], sizeof(chrMQTT_IPv4));
  prefMQTT_IPv4 = myPreferences.getString("MqttIPv4", chrMQTT_IPv4);
  prefMQTT_IPv4.toCharArray(arrMQTT_IPv4_0, sizeof(arrMQTT_IPv4_0));
// MQTT Inputs
  intMQTT_Inputs = arrMQTT_Inputs[0];
  prefMQTT_Inputs = myPreferences.getUInt("MqttInputs", intMQTT_Inputs);
  arrMQTT_Inputs[0] = prefMQTT_Inputs;
  myPreferences.end(); 
  if (C_Debug_Level >= 1) {
    Serial.print("\nPreferences \n");
    Serial.printf("TALLY Mode: %s -> ", chrTallyMode);
    Serial.println(prefTallyMode);
    Serial.printf("ATEM Name: %s -> ", chrATEM_Name);
    Serial.println(prefATEM_Name);
    Serial.printf("ATEM IPv4: %s -> ", chrATEM_IPv4);
    Serial.println(prefATEM_IPv4);
    Serial.printf("ATEM Inputs: %d -> ", intATEM_Inputs);
    Serial.println(prefATEM_Inputs);
    Serial.printf("MQTT Name: %s -> ", chrMQTT_Name);
    Serial.println(prefMQTT_Name);
    Serial.printf("MQTT IPv4: %s -> ", chrMQTT_IPv4);
    Serial.println(prefMQTT_IPv4);
    Serial.printf("MQTT Inputs: %d -> ", intMQTT_Inputs);
    Serial.println(prefMQTT_Inputs);
  }
}

void savePreferences() {
  int i;
  char oldPref[16];
  char newPref[16];
  Serial.println("Save Preferences");
  myPreferences.begin(C_Pref_Section, false);
// Tally Mode 
  prefTallyMode.toCharArray(oldPref, sizeof(oldPref));
  strlcpy(newPref, arrTallyMode_0, sizeof(newPref));
  i = strcmp(oldPref, newPref);
  Serial.printf("TALLY Mode %d: %s(%d) -> %s(%d)\n", i, oldPref, sizeof(oldPref), newPref, sizeof(newPref));
  if (i != 0) myPreferences.putString("TallyMode", arrTallyMode_0);
// ATEM Name
  prefATEM_Name.toCharArray(oldPref, sizeof(oldPref));
  strlcpy(newPref, arrATEM_Name_0, sizeof(newPref));
  i = strcmp(oldPref, newPref);
  Serial.printf("ATEM Name %d: %s -> %s\n", i, oldPref, newPref);
  if (i != 0) myPreferences.putString("AtemName", arrATEM_Name_0);
// ATEM IPv4
  prefATEM_IPv4.toCharArray(oldPref, sizeof(oldPref));
  strlcpy(newPref, arrATEM_IPv4_0, sizeof(newPref));
  i = strcmp(oldPref, newPref);
  Serial.printf("ATEM IPv4 %d: %s(%d) -> %s(%d)\n", i, oldPref, sizeof(oldPref), newPref, sizeof(newPref));
  if (i != 0) myPreferences.putString("AtemIPv4", arrATEM_IPv4_0);
// ATEM Inputs
  intATEM_Inputs = arrATEM_Inputs[0];
  Serial.printf("ATEM Inputs: %d -> %d\n", prefATEM_Inputs, intATEM_Inputs);
  if (prefATEM_Inputs != intATEM_Inputs) myPreferences.putUInt("AtemInputs", intATEM_Inputs);
// MQTT Name
  prefMQTT_Name.toCharArray(oldPref, sizeof(oldPref));
  strlcpy(newPref, arrMQTT_Name_0, sizeof(newPref));
  i = strcmp(oldPref, newPref);
  Serial.printf("MQTT Name %d: %s -> %s\n", i, oldPref, newPref);
  if (i != 0) myPreferences.putString("MqttName", arrMQTT_Name_0);
// MQTT IPv4
  prefMQTT_IPv4.toCharArray(oldPref, sizeof(oldPref));
  strlcpy(newPref, arrMQTT_IPv4_0, sizeof(newPref));
  i = strcmp(oldPref, newPref);
  Serial.printf("MQTT IPv4 %d: %s(%d) -> %s(%d)\n", i, oldPref, sizeof(oldPref), newPref, sizeof(newPref));
  if (i != 0) myPreferences.putString("MqttIPv4", arrMQTT_IPv4_0);
// MQTT Inputs
  intMQTT_Inputs = arrMQTT_Inputs[0];
  Serial.printf("MQTT Inputs: %d -> %d\n", prefMQTT_Inputs, intMQTT_Inputs);
  if (prefMQTT_Inputs != intMQTT_Inputs) myPreferences.putUInt("MqttInputs", intMQTT_Inputs);
  myPreferences.end(); 
}

// Loop
void drawTally() {
  bool isPreviewTally = false;
  if (intCameraNumber == intPreviewInput) isPreviewTally = true;
  bool isProgramTally = false;
  if (intCameraNumber == intProgramInput) isProgramTally = true;
  if ((intOrientation == intOrientationPrevious) && (intCameraNumber == intCameraNumberPrevious) && (isPreviewTally == isPreviewTallyPrevious) && (isProgramTally == isProgramTallyPrevious)) return; // nothing changed?
  isPreviewTallyPrevious = isPreviewTally;
  isProgramTallyPrevious = isProgramTally;
  strLcdText = String(intCameraNumber);
  if (isProgramTally && isPreviewTally) { // program AND preview
    drawLabel(strLcdText, GREEN, RED, RED);
    return;
  } 
  if (isProgramTally) { // only program
    drawLabel(strLcdText, BLACK, RED, RED);
    return;
  } 
  if (isPreviewTally) { // only preview
    drawLabel(strLcdText, BLACK, GREEN, GREEN);
    return;
  } // neither
  unsigned int iColor = GRAY;
  if (isLedHatMode == true) iColor = WHITE;
  drawLabel(strLcdText, iColor, BLACK, BLACK);
  if (C_Debug_Level >= 1) {
    Serial.printf("LED-HAT Color: %X\n", iColor);
  }
}

void drawBroker() {
  setLED(BLACK);
  if (intSetupPage != 0) return;
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setRotation(intOrientation);
  int intTextDatum = M5.Lcd.getTextDatum();
  M5.Lcd.setTextDatum(MC_DATUM);
  strLcdText = String(intPreviewInput);
  if (intPreviewInput > arrATEM_Inputs[0]) strLcdText = "-";
  M5.Lcd.setTextColor(GREEN);
  if (intOrientation % 2 == 0) {
    M5.Lcd.drawString(strLcdText, M5.Lcd.width() / 2, M5.Lcd.height() / 4, 8);
  } else {
    M5.Lcd.drawString(strLcdText, M5.Lcd.width() / 4, M5.Lcd.height() / 2, 8);
  }
  strLcdText = String(intProgramInput);
  if (intProgramInput > arrATEM_Inputs[0]) strLcdText = "-";
  M5.Lcd.setTextColor(RED);
  if (intOrientation % 2 == 0) {
    M5.Lcd.drawString(strLcdText, M5.Lcd.width() / 2, M5.Lcd.height() * 3 / 4, 8);
  } else {
    M5.Lcd.drawString(strLcdText, M5.Lcd.width() * 3 / 4, M5.Lcd.height() / 2, 8);
  }
  M5.Lcd.setTextDatum(intTextDatum);
}

void drawLabel(String labelText, unsigned long int labelColor, unsigned long int screenColor, int iColor) {
  setLED(iColor);
  if (intSetupPage != 0) return;
  M5.Lcd.fillScreen(screenColor);
  M5.Lcd.setRotation(intOrientation);
  M5.Lcd.setTextColor(labelColor, screenColor);
  int intTextDatum = M5.Lcd.getTextDatum();
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString(labelText, M5.Lcd.width() / 2, M5.Lcd.height() / 2, 8);
  M5.Lcd.setTextDatum(intTextDatum);
}

void setM5Orientation() {
  float accX = 0, accY = 0, accZ = 0;
  #ifdef M5STICKCPLUS 
  M5.Imu.getAccelData(&accX, &accY, &accZ);
  #else
  M5.MPU6886.getAccelData(&accX, &accY, &accZ);
  #endif
  //Serial.printf("%.2f   %.2f   %.2f \n",accX * 1000, accY * 1000, accZ * 1000);
  if (accZ < .9) {
    if (accX > .6) {
      intOrientation = 1;
    } else if (accX < .4 && accX > -.5) {
      if (accY > 0) {
        intOrientation = 0;
      } else {
        intOrientation = 2;
      }
    } else {
      intOrientation = 3;
    }
  }
  if (intOrientation != intOrientationPrevious) {
    if (C_Debug_Level >= 2) {
      Serial.printf("M5 Orientation changed to %d\n", intOrientation);
    }
    M5.Lcd.setRotation(intOrientation);
  }
}

String myWebServerIndex = 
"<form method='POST' action='/update' enctype='multipart/form-data'>"
"<br>"
"<input type='file' name='update'>"
"<br><br>"
"<input type='submit' value='Update'>"
"</form>";

void startWebServer() {
  myWebServer.on("/", HTTP_GET, []() {
    myWebServer.sendHeader("Connection", "close");
    myWebServer.send(200, "text/html", myWebServerIndex);
  });
  /*handling uploading firmware file */
  myWebServer.on("/update", HTTP_POST, []() {
    myWebServer.sendHeader("Connection", "close");
    myWebServer.send(200, "text/plain", (Update.hasError()) ? "FEHLER" : "OK");
//    intSetupPage = 1;
    waitButton(0);
    restartESP();
  }, []() {
    HTTPUpload& myHTTPUpload = myWebServer.upload();
    if (myHTTPUpload.status == UPLOAD_FILE_START) {
      if (C_Debug_Level >= 1) Serial.printf("Update: %s\n", myHTTPUpload.filename.c_str());
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.println();
      M5.Lcd.println(myHTTPUpload.filename.c_str());
      setLED(YELLOW);
//      digitalWrite(LED_PIN, LOW);
      intUpdateMillis = 0;
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        if (C_Debug_Level >= 1) Update.printError(Serial);
        M5.Lcd.setTextColor(RED);
        Update.printError(M5.Lcd);
      }
    } else if (myHTTPUpload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (millis() - intUpdateMillis > 1000) {
        if (C_Debug_Level >= 2) Serial.print(".");
        M5.Lcd.print(".");
        intUpdateMillis = millis();
      }
      if (Update.write(myHTTPUpload.buf, myHTTPUpload.currentSize) != myHTTPUpload.currentSize) {
        if (C_Debug_Level >= 1) Update.printError(Serial);
        M5.Lcd.setTextColor(RED);
        Update.printError(M5.Lcd);
      }
    } else if (myHTTPUpload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        if (C_Debug_Level >= 1) Serial.printf("\n%u Byte Ok\nWait for Reboot...\n", myHTTPUpload.totalSize);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.printf("\n\n %u Byte ", myHTTPUpload.totalSize);
        M5.Lcd.setTextColor(GREEN);
        M5.Lcd.print("Ok \n\n");
        M5.Lcd.setTextColor(WHITE);
      } else {
        if (C_Debug_Level >= 1) Update.printError(Serial);
        M5.Lcd.setTextColor(RED);
        Update.printError(M5.Lcd);
      }
    }
  });
  myWebServer.begin();
//  if (intSetupPage == 0) return;
  if (C_Debug_Level >= 2) {
    Serial.print("HTTP Update Server started");
    Serial.println();
  }
}

void stopWebServer() {
  myWebServer.stop(); 
  if (intSetupPage == 0) return;
  if (C_Debug_Level >= 2) {
    Serial.print("HTTP Update Server stopped");
    Serial.println();
  }
}
/*
void crashArray() {
  return;
  strlcpy(chrATEM_Name, arrATEM_Name[1], sizeof(chrATEM_Name));
  Serial.printf("arrATEM_Name[0] = %s[%d]\n", arrATEM_Name[0], strlen(arrATEM_Name[0]));
  Serial.printf("arrATEM_Name[1] = %s[%d]\n", arrATEM_Name[1], strlen(arrATEM_Name[1]));
  Serial.printf("chrATEM_Name = %s[%d]\n", chrATEM_Name, strlen(chrATEM_Name));
  strlcpy(arrATEM_Name[0], chrATEM_Name, (strlen(chrATEM_Name) + 1));
}
*/
// EOF
