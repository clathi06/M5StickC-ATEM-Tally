// http://librarymanager/All#M5StickC https://github.com/m5stack/M5StickC
#define M5STICKCPLUS
#ifdef M5STICKCPLUS
#include <M5StickCPlus.h>
#else
#include <M5StickC.h>
#endif
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager

// Download these from here
// https://github.com/kasperskaarhoj/SKAARHOJ-Open-Engineering/tree/master/ArduinoLibs
#include <SkaarhojPgmspace.h>
#include <ATEMbase.h>
#include <ATEMstd.h>

#define LED_PIN 10

// You can customize the colors if you want
// http://www.barth-dev.de/online/rgb565-color-picker/
#define GRAY   0x0841 //   8   8  8
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
const char* C_Pgm_Version = "2021-04-05";
const char* C_AP_SSID = "ATEM-Tally@M5StickC";
char chrWiFiSSID[41] = "SSID";
//char chrWiFiSSID[41] = "Technik@Halle";
char chrWiFi_Pwd[41] = "";
char chrHostName[12] = "tl-12-34-56";

// Setup 0
Preferences myPreferences;
unsigned int prefBootCount;
const char* C_Pref_Section = "Tally";
String prefAtemName;
String prefAtemIPv4;
unsigned int prefAtemInputs;

// Setup 1
int intTimeOutMin; // Timeout in Minuten
int intTimeOutSec; // zusätzlicher Timeout in Sekunden
int intTimeOut;

// Setup 2 und 5
ATEMstd myAtemSwitcher;
// Define the Hostname and/or IP address of your ATEM switcher
const int C_AtemNameSize = 5;
char* arr_AtemName[] = {"", "atem", "atem-mini", "atem-tvs", "atem-tvs-hd"} ;
char chrAtemName[16];
char arr_AtemName_0[16];
char* arr_AtemIPv4[] = {"0.0.0.0", "192.168.10.240", "192.168.10.240", "192.168.118.81", "192.168.118.82"};
char chrAtemIPv4[16];
char arr_AtemIPv4_0[16];
unsigned int arr_AtemInputs[] = {1, 8, 4, 6, 8};
unsigned int intAtemInputs;
//unsigned int intrun_AtemInputs;
IPAddress ipAtem;
IPAddress ipAtemDNS;
//IPAddress ipAtem(192, 168, 118, 240);
//IPAddress ipAtem(2, 0, 0, 8);
int intAtemNr;
int intAtemDNS;

// Setup 3
WiFiManager wifiManager;
WiFiManagerParameter wmp_atem_name("atem_name", "ATEM Name");
WiFiManagerParameter wmp_atem_ipv4("atem_ipv4", "ATEM IPv4");
bool isConfigPortalStarted;
bool isConfigPortalActive;

// Setup 4
WebServer myWebServer(80);

int intSetupCount = 0;
int intSetupLoop = 0;
int intSetupPage = 1;

bool isAutoOrientation = 0;
int intOrientation = 0;
int intOrientationPrevious = 0;
unsigned long intOrientationMillisPrevious = millis();
unsigned long lngButtonAMillis = 0;
unsigned long lngButtonBMillis = 0;
int intUpdateMillis = 0;

String strLcdSize;
String strLcdText = "1";

int intCameraNumber = 1;
int intCameraNumberPrevious = 0;
int intPreviewTallyPrevious = 0;
int intProgramTallyPrevious = 0;

void setup() {
// Serial
  Serial.begin(115200);
// M5
  M5.begin();
  #if defined(M5STICKC) 
  M5.MPU6886.Init();
  #endif
  #ifdef M5STICKCPLUS)
  M5.Imu.Init();
  #endif
  pinMode(LED_PIN, OUTPUT);
  strLcdSize = String(M5.Lcd.width()) + "*" + String(M5.Lcd.height());
// ATEM Array
  strlcpy(arr_AtemName_0, arr_AtemName[0], sizeof(arr_AtemName_0));
  arr_AtemName[0] = arr_AtemName_0; // Tricky: Zeiger umbiegen
  strlcpy(arr_AtemIPv4_0, arr_AtemIPv4[0], sizeof(arr_AtemIPv4_0));
  arr_AtemIPv4[0] = arr_AtemIPv4_0;
// Preferences
  myPreferences.begin("ESP32", false); 
  prefBootCount = myPreferences.getUInt("BootCount", 0);
  prefBootCount++;
  myPreferences.putUInt("BootCount", prefBootCount);
  myPreferences.end();
  getPreferences(); 
// WiFi
  String strHostName = WiFi.macAddress();
  strHostName.replace(":", "-");
  strHostName.toLowerCase();
  strHostName = "tl-" + strHostName.substring(9);
  strHostName.toCharArray(chrHostName, 12);
  if (C_Debug_Level >= 1) {
    Serial.print("WiFi Settings \n");
    Serial.print("WiFi MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.print("WiFi Hostname: ");
    Serial.println(chrHostName);
  }
//  WiFiManager wifiManager;
  if (C_Debug_Level < 4) wifiManager.setDebugOutput(false); 
  wifiManager.addParameter(&wmp_atem_name);
  wifiManager.addParameter(&wmp_atem_ipv4);  
  wifiManager.setHostname(chrHostName);
// Timeout
  intTimeOutMin = 9 - C_Debug_Level;
  if (intTimeOutMin < 0) intTimeOutMin = 0;
  intTimeOutSec = 0;
  if (intTimeOutMin == 0) intTimeOutSec = 10;
  intTimeOut = (intTimeOutMin * 60) + intTimeOutSec;
  return;
// GPIO
  pinMode(26, INPUT); // PIN  (INPUT, OUTPUT, ANALOG)
  pinMode(36, INPUT); // PIN  (INPUT,       , ANALOG)
  pinMode( 0, INPUT); // PIN  (INPUT, OUTPUT,       )
  pinMode(32, INPUT); // GROVE(INPUT, OUTPUT, ANALOG)
  pinMode(33, INPUT); // GROVE(INPUT, OUTPUT, ANALOG)
}

void loop() {
  checkEvents();

  int intPreviewTally = myAtemSwitcher.getPreviewTally(intCameraNumber);
  int intProgramTally = myAtemSwitcher.getProgramTally(intCameraNumber);
  
  if ((intOrientation != intOrientationPrevious) || (intCameraNumber != intCameraNumberPrevious) || (intPreviewTallyPrevious != intPreviewTally) || (intProgramTallyPrevious != intProgramTally)) { // changed?
    strLcdText = String(intCameraNumber);
    if (intProgramTally && intPreviewTally) { // program AND preview
      drawLabel(strLcdText, GREEN, RED, LOW);
    } else if (intProgramTally && !intPreviewTally) { // only program
      drawLabel(strLcdText, BLACK, RED, LOW);
    } else if (intPreviewTally && !intProgramTally) { // only preview
      drawLabel(strLcdText, BLACK, GREEN, HIGH);
    } else if (!intPreviewTally || !intProgramTally) { // neither
      drawLabel(strLcdText, GRAY, BLACK, HIGH);
//      drawLabel(strLcdText, WHITE, BLACK, HIGH);
    }
  }

  intOrientationPrevious  = intOrientation;
  intCameraNumberPrevious = intCameraNumber;
  intPreviewTallyPrevious = intPreviewTally;
  intProgramTallyPrevious = intProgramTally;

}

void checkEvents() {
  // Check for M5Stick Buttons
  checkM5Events();
  // Check for Setup 
  if (intSetupPage != 0) checkSetup();
  if (intSetupPage != 0) return;
  // Check for WiFi
  checkLoop();
  // Check for packets, respond to them etc. Keeping the connection alive!
  myAtemSwitcher.runLoop();
}

void checkM5Events() {
  M5.update();
  if (isAutoOrientation) {
    if (millis() - intOrientationMillisPrevious >= 500 ) {
      setM5Orientation();
      intOrientationMillisPrevious = millis();
    }
  }
  if (M5.BtnA.wasPressed()) {
    if (C_Debug_Level >= 3) Serial.println("M5.BtnA.wasPressed");
    intCameraNumber = (intCameraNumber % intAtemInputs) + 1;
    if (C_Debug_Level >= 2) Serial.printf("ATEM next Camera Number: %d\n", intCameraNumber);
    lngButtonAMillis = millis();
  }
  if (M5.BtnA.isPressed() && lngButtonAMillis != 0 && millis() - lngButtonAMillis >= 500 ) {
    if (C_Debug_Level >= 3) Serial.println("M5.BtnA.isPressed");
    restartESP();
    intSetupPage = 1;
    return;
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

void checkLoop() {
  if (WiFi.status() == WL_CONNECTED) return;
  waitWiFi();
  initM5Lcd();
  intCameraNumberPrevious = 0;
}

void flashLED() {
  clearM5Lcd(0);
  for (int i = 6; i > 0; i--) {
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(i * 100);
  }
}

void restartESP() {
  digitalWrite(LED_PIN, LOW);
  stopWiFi();
  if (C_Debug_Level >= 3) Serial.println("M5 Restart");
  Serial.flush();
  flashLED();
  ESP.restart();
}

void stopESP() {
  if (C_Debug_Level >= 1) Serial.println("M5 PowerOff");
  Serial.flush();
  flashLED();
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
    iTimeOut = iTimeOut - 2;
    if (C_Debug_Level >= 2) Serial.print(".");
    if (C_Debug_Level >= 3) Serial.print(WiFi.status());
    drawLabel(String(iTimeOut / 60 + 1), RED, BLACK, LOW);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    drawLabel(strLcdText, WHITE, BLACK, HIGH);
    digitalWrite(LED_PIN, HIGH);
    intMillis = millis();
    lngMillis = millis();
    while (millis() - lngMillis < 2000) {
      if (WiFi.status() == WL_CONNECTED) lngMillis = 0;
      if (M5.BtnA.isPressed() && millis() - intMillis >= 500 ) {
        if (C_Debug_Level >= 3) Serial.println("\nM5.BtnA.isPressed");
        restartESP();
      }
      M5.update();
      if (isConfigPortalActive == true) wifiManager.process();
    }
    intLoop++;
    if (intLoop % 30 == 0) {
      if (C_Debug_Level >= 2) Serial.println();
    }
  }
  if (C_Debug_Level >= 2) Serial.println("WiFi connected");
  return true;
}

void checkSetup() {
  intSetupCount++;
  if (C_Debug_Level >= 2) {
    Serial.printf("\nStarting Setup Mode %d at Count %d\n", intSetupPage, intSetupCount);
  }
  initM5Lcd();
  intSetupLoop = 0;
  if (intSetupPage == 1) stopWiFi();
  while (intSetupPage == 1) {
    printM5Info();
    waitButton(intSetupPage);
    if (intSetupPage < 1) stopESP();
    intSetupLoop++;
  }
  intSetupLoop = 0;
  while (intSetupPage == 2) {
    getAtemName();
    waitButton(intSetupPage);
    if (intSetupPage < 2) {
      clearPreferences();
      return;
    }
    if (intSetupPage > 2) setAtemName();
    intSetupLoop++;
  }
  while (intSetupPage == 3) {
    startWiFiManager();
    waitButton(intSetupPage);
    if (intSetupPage < 3) {
      if (C_Debug_Level >= 1) Serial.println("WM.resetSettings");
      wifiManager.resetSettings();
      return;
    }
  }
  startWiFi();
  while (intSetupPage == 4) {
    startWebServer();
    waitButton(intSetupPage);
    stopWebServer();
    if (intSetupPage < 4) return;
    if (intSetupPage == 4) delay(1000);
  }
  intAtemDNS = 0;
  while (intSetupPage == 5) {
    getAtemDNS();
    waitButton(intSetupPage);
    if (intSetupPage < 5) return;
  }
  startAtem();
  intCameraNumberPrevious = 0;
  if (C_Debug_Level >= 2) {
    Serial.println("ATEM last Camera Number: 0");
  }
  intCameraNumber = 1;
  if (C_Debug_Level >= 1) {
    Serial.println("ATEM next Camera Number: 1");
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
    case 3:
      if (isConfigPortalActive) {
        M5.Lcd.print(" auf Eingaben und/oder ");
      } else {
        M5.Lcd.print(" Weiter mit ");
      }
      break;
    case 4:
      M5.Lcd.print(" HTTP Update oder ");
      break;
    default:
      M5.Lcd.print(" Auswahl oder ");
      break;
  }
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.println("M5");
  M5.Lcd.setTextColor(WHITE);
  while (M5.BtnA.wasPressed() == false) {
    if (millis() - lngMillis >= lngIntervall) {
      lngMillis = millis();
      if (intLedState == HIGH) {
        intLedState = LOW;
        lngIntervall = 10000;
      } else {
        intLedState = HIGH;     
        lngIntervall = 50000;
        iTimeOut = iTimeOut - 60;
      }
      if (iTimeOut <= 0) {
        if (C_Debug_Level >= 1) Serial.println("M5 Timeout");
        intSetupPage--;
        if (intSetupPage <= 0) stopESP();
        restartESP();
      }
      digitalWrite(LED_PIN, intLedState);
    }
    M5.update();
    if (checkBtnB(i1) == true) return;
    if (isConfigPortalActive == true) wifiManager.process(); 
    if (i1 >= 4) myWebServer.handleClient(); 
    if (i1 >= 5) myAtemSwitcher.runLoop();
  }
  if (C_Debug_Level >= 3) Serial.println("M5.BtnA.wasPressed");
  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.fillScreen(BLACK);
  lngMillis = millis();
  while (M5.BtnA.isPressed() == true) {
    if (millis() - lngMillis >= 500) {
      if (C_Debug_Level >= 3) Serial.println("M5.BtnA.isPressed");
//      if (C_Debug_Level >= 3) Serial.println("M5 GPIO39=" + String(digitalRead(39)));
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
    case 5:
      return true;
    default:
      return false;
  }
}

void initM5Lcd() {
  // Initialize Display
  if (C_Debug_Level >= 2) Serial.println("M5 initializing Display");
  // Lcd display test
  digitalWrite(LED_PIN, LOW);
  M5.Lcd.fillScreen(BLACK);
  delay(1000);
  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.fillScreen(WHITE);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  M5.Lcd.fillScreen(RED);
  delay(1000);
  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.fillScreen(YELLOW);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  M5.Lcd.fillScreen(GREEN);
  delay(1000);
  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setRotation(1); //drehen des Displays
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
}

void clearM5Lcd(int i1) {
  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.fillScreen(BLACK);
  if (i1 == 0) return;
  delay(100);
  M5.Lcd.setCursor(0, 0);
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(); 
  #endif
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.printf(" %d\.%d", intSetupCount, intSetupPage);
}

void printM5Info() {
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
  clearM5Lcd(intSetupPage);
  M5.Lcd.printf(" %s %s\n\n", C_Pgm_Name, C_Pgm_Version);
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

void getAtemName() {
  if (intSetupLoop == 0) {
    intAtemNr = 0;
    prefAtemName.toCharArray(arr_AtemName_0, sizeof(arr_AtemName_0));
    prefAtemIPv4.toCharArray(arr_AtemIPv4_0, sizeof(arr_AtemIPv4_0));
    arr_AtemInputs[0] = prefAtemInputs;
  }
  if (intSetupLoop != 0) {
    intAtemNr++;
    if (intAtemNr >= C_AtemNameSize) intAtemNr = 0;
  }
  printAtemNameInfo1();
  for (int i = 0; i < C_AtemNameSize; i++) {
    if (printAtemNameInfo2(i) == false) {
      if (intAtemNr == 0) intAtemNr++;
    }
  }
}

void setAtemName() {
  if (intAtemNr != 0) {
    strlcpy(arr_AtemName_0, arr_AtemName[intAtemNr], sizeof(arr_AtemName_0));
    strlcpy(arr_AtemIPv4_0, arr_AtemIPv4[intAtemNr], sizeof(arr_AtemIPv4_0));
    intAtemInputs = arr_AtemInputs[intAtemNr];
    arr_AtemInputs[0] = intAtemInputs;
  }
/*  if (intAtemNr != 0) {
    strlcpy(arr_AtemName[0], chrAtemName, sizeof(chrAtemName));
    strlcpy(arr_AtemIPv4[0], chrAtemIPv4, sizeof(chrAtemIPv4));
    arr_AtemInputs[0] = intAtemInputs;
    arr_AtemName[0] = chrAtemName;
    arr_AtemIPv4[0] = chrAtemIPv4;*/
  if (C_Debug_Level >= 1) {
    Serial.printf("ATEM Nr. %d %s ", intAtemNr, arr_AtemName_0);
    Serial.printf("on %s with %d Inputs selected\n", arr_AtemIPv4_0, intAtemInputs);
  }
}

void printAtemNameInfo1() {
  if (C_Debug_Level >= 2) Serial.printf("ATEM new Number: %d\n", intAtemNr);
  clearM5Lcd(intSetupPage);
  M5.Lcd.print(" ATEM Auswahl mit ");
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.print("BtnB\n\n");
}

bool printAtemNameInfo2(int i1) {
//  String sText = arr_AtemName[i1];
//  int iLength = sText.length();
  strlcpy(chrAtemName, arr_AtemName[i1], sizeof(chrAtemName));
  strlcpy(chrAtemIPv4, arr_AtemIPv4[i1], sizeof(chrAtemIPv4));
  intAtemInputs = arr_AtemInputs[i1];
  unsigned int iLen = strlen(chrAtemName);
  if ((C_Debug_Level >= 1) && (intSetupLoop == 0)) {
    Serial.printf("ATEM Nr. %d %s[%d] ", i1, chrAtemName, iLen);
    Serial.printf("on %s with %d Inputs\n", chrAtemIPv4, intAtemInputs);
  }
  M5.Lcd.setTextColor(WHITE);
  if (iLen == 0) {
    return false;
  }
  if (i1 == intAtemNr) M5.Lcd.setTextColor(GREEN);
  M5.Lcd.printf(" %d %s ", intAtemInputs, chrAtemName);
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(chrAtemIPv4);
  #endif
  M5.Lcd.println();
  return true;
}

void startWiFiManager() {
  if ((intSetupPage > 3) && (WiFi.status() == WL_CONNECTED)) return;
  wifiManager.setAPStaticIPConfig(IPAddress(192,168,5,1), IPAddress(192,168,5,1), IPAddress(255,255,255,0));
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setConfigPortalTimeout(intTimeOut);
  wifiManager.setAPCallback(wm_configModeCallback);
  wifiManager.setSaveConfigCallback(wm_saveConfigCallback);
  printManagerInfo1();
// id/name, placeholder/prompt, default, length
//  strlcpy(chrAtemName, arr_AtemName[0], sizeof(chrAtemName));
  wmp_atem_name.setValue(arr_AtemName_0, (sizeof(arr_AtemName_0) - 1));
//  strlcpy(chrAtemIPv4, arr_AtemIPv4[0], sizeof(chrAtemIPv4));
  wmp_atem_ipv4.setValue(arr_AtemIPv4_0, (sizeof(arr_AtemIPv4_0) - 1));
//  wifiManager.autoConnect(C_AP_SSID);
//  wifiManager.startConfigPortal(C_AP_SSID);
  if (intSetupCount == 1) {
    isConfigPortalStarted = false;
    isConfigPortalActive = false;
    wifiManager.autoConnect(C_AP_SSID);
  } else {
    isConfigPortalStarted = true;
    isConfigPortalActive = false;
    wifiManager.startConfigPortal(C_AP_SSID);
  }
  waitWiFi();
  if (isConfigPortalStarted == true) {
    waitButton(intSetupPage);
    if (isConfigPortalActive == true) wifiManager.stopConfigPortal();
    delay(1000);
  }
  isConfigPortalActive = false;
  printManagerInfo2();
}

void printManagerInfo1() {
  Serial.println("WM starting with");
  Serial.printf("WM TimeOut %d:%02d\n", intTimeOutMin, intTimeOutSec);
  Serial.printf("WM ATEM Name %s\n", arr_AtemName_0);
  Serial.printf("WM ATEM IPv4 %s\n", arr_AtemIPv4_0);
  clearM5Lcd(intSetupPage);
  M5.Lcd.print(" WiFi Manager\n\n");
  M5.Lcd.printf(" TimeOut %d:%02d Minute(n)\n", intTimeOutMin, intTimeOutSec);
}

void wm_configModeCallback (WiFiManager *myWiFiManager) {
  isConfigPortalActive = true;
  Serial.println("WM Entered config mode on");
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.println(WiFi.softAPIP());
  clearM5Lcd(intSetupPage);
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
  strcpy(arr_AtemName_0, wmp_atem_name.getValue());
//  strcpy(chrAtemName, wmp_atem_name.getValue());
//  strlcpy(arr_AtemName[0], chrAtemName, sizeof(chrAtemName));
  strcpy(arr_AtemIPv4_0, wmp_atem_ipv4.getValue());
//  strcpy(chrAtemIPv4, wmp_atem_ipv4.getValue());
//  arr_AtemIPv4[0] = chrAtemIPv4;
//  strlcpy(arr_AtemIPv4[0], chrAtemIPv4, sizeof(chrAtemIPv4));
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
    Serial.println(arr_AtemName_0);
    Serial.print("ATEM IPv4 ");
    Serial.println(arr_AtemIPv4_0);
  }
  clearM5Lcd(intSetupPage);
  M5.Lcd.print(" WiFi Manager\n\n");
  M5.Lcd.print(" SSID ");
  M5.Lcd.println(chrWiFiSSID);
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(); 
  #endif
  M5.Lcd.print(" IPv4 ");
  M5.Lcd.println(WiFi.localIP());
  M5.Lcd.println();
  M5.Lcd.printf(" ATEM %s\n", arr_AtemName_0);
  #ifdef M5STICKCPLUS 
    M5.Lcd.println(); 
  #endif
  M5.Lcd.print(" IPv4 ");
  if (!ipAtem.fromString(arr_AtemIPv4_0)) M5.Lcd.setTextColor(RED);
  M5.Lcd.println(arr_AtemIPv4_0);
}

void startWiFi() {
  if ((intSetupPage > 4) && (WiFi.status() == WL_CONNECTED)) return;
  if (intSetupPage > 4) intSetupPage = 4;
  printWiFiInfo1();
  int intLoop = 0;
  int intMillis = 0;
  int iTimeOut = intTimeOut;
  unsigned long lngMillis = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (iTimeOut <= 0) {
      restartESP();
    }
    iTimeOut = iTimeOut - 2;
    if (C_Debug_Level >= 2) Serial.print(".");
    if (C_Debug_Level >= 3) Serial.print(WiFi.status());
    drawLabel(strLcdText, RED, BLACK, LOW);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    drawLabel(strLcdText, GRAY, BLACK, HIGH);
    digitalWrite(LED_PIN, HIGH);
    intMillis = millis();
    lngMillis = millis();
    while (millis() - lngMillis < 2000) {
      if (WiFi.status() == WL_CONNECTED) lngMillis = 0;
      if (M5.BtnA.isPressed() && millis() - intMillis >= 500 ) {
        if (C_Debug_Level >= 3) Serial.println("M5.BtnA.isPressed");
        restartESP();
      }
      M5.update();
    }
    intLoop++;
    if (intLoop % 30 == 0) {
      if (C_Debug_Level >= 2) Serial.println();
    }
  }
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
  if (intSetupPage != 4) return;
  clearM5Lcd(intSetupPage);
  M5.Lcd.print(" WiFi Einstellungen\n\n");
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
  if (intSetupPage != 4) return;
  M5.Lcd.print(" IPv4 ");
  M5.Lcd.println(ip);
  M5.update();
}

void getAtemDNS() {
  // Connecting AtemSwitcher
  int rcDNS;
  if (intAtemDNS == 0) {
//    intAtemInputs = arr_AtemInputs[0];
//    strlcpy(chrAtemName, arr_AtemName[0], sizeof(chrAtemName));
//    strlcpy(chrAtemIPv4, arr_AtemIPv4[0], sizeof(chrAtemIPv4));
//    rcDNS = WiFi.hostByName(chrAtemName, ipAtemDNS);
    rcDNS = WiFi.hostByName(arr_AtemName_0, ipAtemDNS);
    if (rcDNS != 1) intAtemDNS++;
    printAtemDNSInfo1(rcDNS);
  }
  intAtemDNS++;
  if (intAtemDNS >= 3) intAtemDNS = 1;
  if (intAtemDNS == 1) {
    ipAtem = ipAtemDNS;
  } else {
    ipAtem.fromString(arr_AtemIPv4_0);
  }
  printAtemDNSInfo2(intAtemDNS, rcDNS);
}

void printAtemDNSInfo1(int i1) {
  if (C_Debug_Level >= 1) {
    Serial.print("ATEM DNS: ");
    Serial.println(i1); 
    Serial.print("ATEM DNS IPv4: ");
    Serial.println(ipAtemDNS);
  }
}

void printAtemDNSInfo2(int i1, int i2) {
  if (C_Debug_Level >= 2) {
    Serial.print("ATEM DNS IPv4: ");
    Serial.print(ipAtem);
    Serial.println(" selected");
  }
  clearM5Lcd(intSetupPage);
  M5.Lcd.print(" ATEM Einstellungen\n\n");
  M5.Lcd.printf(" NAME %s\n\n", arr_AtemName_0);
  M5.Lcd.print("  DNS ");
  if (i1 == 1) M5.Lcd.setTextColor(GREEN);
  if ((i1 == 1) && (i2 != 1)) M5.Lcd.setTextColor(RED);
  M5.Lcd.println(ipAtemDNS);
  M5.Lcd.println();
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print(" oder ");
  if (i1 != 1) M5.Lcd.setTextColor(GREEN);
  M5.Lcd.println(arr_AtemIPv4_0);
  M5.Lcd.setTextColor(WHITE);
}

void startAtem() {
  String sText = ipAtem.toString();
  sText.toCharArray(arr_AtemIPv4_0, sizeof(arr_AtemIPv4_0));
//  sText.toCharArray(chrAtemIPv4, sizeof(chrAtemIPv4));
//  strlcpy(arr_AtemIPv4[0], chrAtemIPv4, sizeof(chrAtemIPv4));
//  arr_AtemIPv4[0] = chrAtemIPv4;
  myAtemSwitcher.begin(ipAtem);
  if (C_Debug_Level >= 5) myAtemSwitcher.serialOutput(0x80);
  myAtemSwitcher.connect();
  intAtemInputs = arr_AtemInputs[0];
  if (C_Debug_Level >= 1) {
    Serial.printf("Atem.begin done with %d Inputs\n", intAtemInputs);
  }
}

void clearPreferences() {
  if (C_Debug_Level >= 1) Serial.println("Preferences.clear");
  myPreferences.begin(C_Pref_Section, false); 
  myPreferences.clear();  
  prefAtemName = "";
  prefAtemIPv4 = "0.0.0.0";
  prefAtemInputs = 1;
  myPreferences.end(); 
}

void getPreferences() {
  myPreferences.begin(C_Pref_Section, false); 
  strlcpy(chrAtemName, arr_AtemName[0], sizeof(chrAtemName));
  prefAtemName = myPreferences.getString("AtemName", chrAtemName);
  strlcpy(chrAtemIPv4, arr_AtemIPv4[0], sizeof(chrAtemIPv4));
  prefAtemIPv4 = myPreferences.getString("AtemIPv4", chrAtemIPv4);
  intAtemInputs = arr_AtemInputs[0];
  prefAtemInputs = myPreferences.getUInt("AtemInputs", intAtemInputs);
  myPreferences.end(); 
  if (C_Debug_Level >= 1) {
    Serial.print("\nPreferences \n");
    Serial.printf("ATEM Name: %s -> ", chrAtemName);
    Serial.println(prefAtemName);
    Serial.printf("ATEM IPv4: %s -> ", chrAtemIPv4);
    Serial.println(prefAtemIPv4);
    Serial.printf("ATEM Inputs: %d -> ", intAtemInputs);
    Serial.println(prefAtemInputs);
  }
}

void savePreferences() {
  int i;
  char oldPref[16];
  char newPref[16];
  Serial.println("Save Preferences");
  myPreferences.begin(C_Pref_Section, false); 
  prefAtemName.toCharArray(oldPref, sizeof(oldPref));
//  strlcpy(newPref, arr_AtemName[0], sizeof(newPref));
  strlcpy(newPref, arr_AtemName_0, sizeof(newPref));
  i = strcmp(oldPref, newPref);
  Serial.printf("ATEM Name %d: %s(%d) -> %s(%d)\n", i, oldPref, sizeof(oldPref), newPref, sizeof(newPref));
  if (i != 0) myPreferences.putString("AtemName", newPref);
  prefAtemIPv4.toCharArray(oldPref, sizeof(oldPref));
//  strlcpy(newPref, arr_AtemIPv4[0], sizeof(newPref));
  strlcpy(newPref, arr_AtemIPv4_0, sizeof(newPref));
  i = strcmp(oldPref, newPref);
  Serial.printf("ATEM IPv4 %d: %s(%d) -> %s(%d)\n", i, oldPref, sizeof(oldPref), newPref, sizeof(newPref));
  if (i != 0) myPreferences.putString("AtemIPv4", newPref);
  intAtemInputs = arr_AtemInputs[0];
  Serial.printf("ATEM Inputs: %d -> %d\n", prefAtemInputs, intAtemInputs);
  if (prefAtemInputs != intAtemInputs) myPreferences.putUInt("AtemInputs", intAtemInputs);
  myPreferences.end(); 
}

void drawLabel(String labelText, unsigned long int labelColor, unsigned long int screenColor, bool ledValue) {
  digitalWrite(LED_PIN, ledValue);
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
    intSetupPage = 1;
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
      digitalWrite(LED_PIN, LOW);
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
  if (intSetupPage == 0) return;
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

void crashArray() {
  return;
  strlcpy(chrAtemName, arr_AtemName[1], sizeof(chrAtemName));
  Serial.printf("arr_AtemName[0] = %s[%d]\n", arr_AtemName[0], strlen(arr_AtemName[0]));
  Serial.printf("arr_AtemName[1] = %s[%d]\n", arr_AtemName[1], strlen(arr_AtemName[1]));
  Serial.printf("chrAtemName = %s[%d]\n", chrAtemName, strlen(chrAtemName));
  strlcpy(arr_AtemName[0], chrAtemName, (strlen(chrAtemName) + 1));
}

// EOF
