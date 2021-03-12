
// http://librarymanager/All#M5StickC https://github.com/m5stack/M5StickC
#include <M5StickC.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Download these from here
// https://github.com/kasperskaarhoj/SKAARHOJ-Open-Engineering/tree/master/ArduinoLibs
#include <SkaarhojPgmspace.h>
#include <ATEMbase.h>
#include <ATEMstd.h>

// Set this to 1 if you want the intOrientation to update automatically
#define LED_PIN 10
#define AUTOUPDATE_ORIENTATION 0

// You can customize the colors if you want
// http://www.barth-dev.de/online/rgb565-color-picker/
#define GRAY   0x0841 //   8   8  8
#define GREEN  0x0400 //   0 128  0
#define RED    0xF800 // 255   0  0
#define YELLOW 0xFFE0 // 255 255  0
 
// Debuglevel
//const int C_Debug_Level = 0; // nix
//const int C_Debug_Level = 1; // Ausgaben
//const int C_Debug_Level = 2; // plus Eingaben
const int C_Debug_Level = 3; // plus Events
//const int C_Debug_Level = 4; // plus AtemSwitcher.serialOutput

// Put your WiFi SSID and C_Wifi_Pwd here
const char* C_Pgm_Name = "ATEM Tally";
const char* C_Pgm_Version = "v2021-03-12";
const char* C_Wifi_SSID = "Technik@Halle";
const char* C_Wifi_Pwd = "Technik@Halle";

// Define the Hostname and/or IP address of your ATEM switcher
const char* C_Atem_Name = "atem";
//const char* C_Atem_Name = "atem.evkg.local";
//IPAddress ipAtem(192, 168, 10, 240);
IPAddress ipAtem(192, 168, 118, 240);
//IPAddress ipAtem(2, 0, 0, 8);

ATEMstd AtemSwitcher;

int intSetup = 1;

int intOrientation = 0;
int intOrientationPrevious = 0;
int intOrientationMillisPrevious = millis();
int intButtonAMillis = 0;
int intButtonBMillis = 0;

int intCameraNumber = 1;
String strLcdText = "1";

int intCameraNumberPrevious = 0;
int intPreviewTallyPrevious = 0;
int intProgramTallyPrevious = 0;

void setup() {
  Serial.begin(115200);
  //
  M5.begin();
  M5.MPU6886.Init();
  pinMode(LED_PIN, OUTPUT);
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

  int intPreviewTally = AtemSwitcher.getPreviewTally(intCameraNumber);
  int intProgramTally = AtemSwitcher.getProgramTally(intCameraNumber);
  
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
  // Check for Setup and WiFi
  while (intSetup != 0) checkCommunication();
  // Check for OTA-Updates
  ArduinoOTA.handle();
  // Check for packets, respond to them etc. Keeping the connection alive!
  AtemSwitcher.runLoop();
}

void checkM5Events() {
  M5.update();
  if (AUTOUPDATE_ORIENTATION) {
    if (millis() - intOrientationMillisPrevious >= 500 ) {
      setM5Orientation();
      intOrientationMillisPrevious = millis();
    }
  }
  if (M5.BtnA.wasPressed()) {
    if (C_Debug_Level >= 3) Serial.println("M5.BtnA.wasPressed");
    intCameraNumber = (intCameraNumber % 8) + 1;
    if (C_Debug_Level >= 2) Serial.printf("ATEM next Camera Number: %d\n", intCameraNumber);
    intButtonAMillis = millis();
  }
  if (M5.BtnA.isPressed() && intButtonAMillis != 0 && millis() - intButtonAMillis >= 500 ) {
    restartESP();
  }
  if (M5.BtnB.wasPressed()) {
    setM5Orientation();
    intButtonBMillis = millis();
  }
}

void checkCommunication() {
  if ((intSetup == 0) && (WiFi.status() == WL_CONNECTED)) return;
  if (C_Debug_Level >= 2) {
    Serial.printf("Starting Setup with intSetup=%d\n", intSetup);
  }
  if (intSetup == 0) drawLabel(strLcdText, GRAY, BLACK, HIGH);
  if (intSetup != 0) initM5();
  if (intSetup == 1) {
    printM5Info();
    initWiFi();
    waitBtnA(1);
    if (intSetup == 1) return;
  }
  startWiFi();
  if (intSetup == 2) {
    startOTA();
//  printOTAInfo();
    waitBtnA(2);
    if (intSetup == 2) return;
  }
  if (intSetup == 3) {
    startAtem();
    waitBtnA(3);
    if (intSetup == 3) return;
  }
  intCameraNumberPrevious = 0;
  if (C_Debug_Level >= 2) {
    Serial.println("ATEM last Camera Number: 0");
  }
  if (intSetup == 0) return;
  intCameraNumber = 1;
  if (C_Debug_Level >= 1) {
    Serial.println("ATEM next Camera Number: 1");
  }
  intSetup = 0;
}

void restartESP() {
  if (C_Debug_Level >= 3) Serial.println("M5.BtnA.isPressed");
  digitalWrite(LED_PIN, LOW);
  stopWiFi();
  if (C_Debug_Level >= 1) Serial.println("WiFi stopped");
  if (C_Debug_Level >= 3) Serial.println("M5 Restart");
  ESP.restart();
}

void waitBtnA(int int1) {
  int intLedState = HIGH;
  unsigned long lngIntervall = 0;
  unsigned long lngMillis = 0;
//  digitalWrite(LED_PIN, LOW);
  if (C_Debug_Level >= 2) Serial.printf("M5 waiting for BtnA at intSetup=%d\n", intSetup);
  switch (int1) {
    case 1:
      M5.Lcd.println("  USB Update or Press M5");
      break;
    case 2:
      M5.Lcd.println("  OTA Update or Press M5");
      break;
    case 3:
      M5.Lcd.println("Press M5");
      break;
  }
  while (M5.BtnA.wasPressed() == false) {
    if (millis() - lngMillis >= lngIntervall) {
      lngMillis = millis();
      if (intLedState == HIGH) {
        intLedState = LOW;
        lngIntervall = 10000;
      } else {
        intLedState = HIGH;     
        lngIntervall = 50000;
      }
    digitalWrite(LED_PIN, intLedState);
    }
    M5.update();
    if (intSetup >= 2) ArduinoOTA.handle();
    if (intSetup >= 3) AtemSwitcher.runLoop();
  }
  if (C_Debug_Level >= 3) Serial.println("M5.BtnA.wasPressed");
  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.fillScreen(BLACK);
  lngMillis = millis();
  while (M5.BtnA.isPressed() == true) {
    if (millis() - lngMillis >= 500) {
      if (C_Debug_Level >= 3) Serial.println("M5.BtnA.isPressed");
      return;
    }
    M5.update();
  }
  intButtonAMillis = 0;
  intSetup++;
}

void initM5() {
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

void printM5Info() {
  M5.Lcd.fillScreen(BLACK);
  delay(100);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print(" ");
  M5.Lcd.print(C_Pgm_Name);
  M5.Lcd.print(" ");
  M5.Lcd.print(C_Pgm_Version);
  M5.Lcd.print("\n\n"); 
  M5.Lcd.print("   M5 short OK long Setup\n\n"); 
  M5.Lcd.print(" BtnB short Orientation\n\n"); 
  M5.update();
}

void initWiFi() {
  if (intSetup > 1) return;
  WiFi.persistent(false);
  stopWiFi();
  printWiFiInfo0();
}

void stopWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    delay(500);
  }
  WiFi.mode(WIFI_OFF);
  delay(500);
}

void startWiFi() {
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
  int intMillis = 0;
  unsigned long lngMillis = 0;
  if (((intSetup == 0) || (intSetup > 2)) && (WiFi.status() == WL_CONNECTED)) return;
  printWiFiInfo1();
  int intLoop = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (intLoop % 20 == 0) {
      digitalWrite(LED_PIN, LOW);
      stopWiFi();
      digitalWrite(LED_PIN, HIGH);
      delay(1000);
      WiFi.mode(WIFI_STA);
      delay(1000);
      WiFi.begin(C_Wifi_SSID, C_Wifi_Pwd);
      delay(1000);
    }
    if (C_Debug_Level >= 2) Serial.print(".");
    if (C_Debug_Level >= 3) Serial.print(WiFi.status());
    drawLabel(strLcdText, RED, BLACK, LOW);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    drawLabel(strLcdText, GRAY, BLACK, HIGH);
    digitalWrite(LED_PIN, HIGH);
    intMillis = millis();
    lngMillis = millis();
    while (millis() - lngMillis < 3000) {
      if (WiFi.status() == WL_CONNECTED) lngMillis = 0;
      if (M5.BtnA.isPressed() && millis() - intMillis >= 500 ) restartESP();
      M5.update();
    }
    intLoop++;
    if (intLoop % 20 == 0) {
      if (C_Debug_Level >= 2) Serial.println();
    }
  }
  printWiFiInfo2();
}

void printWiFiInfo0() {
  if (C_Debug_Level >= 1) {
    Serial.println("WiFi stopped");
  }
  if (intSetup == 0) return;
  M5.Lcd.println(" WiFi stopped");
  M5.Lcd.println();
  M5.update();
}

void printWiFiInfo1() {
  // Connecting WiFi
  if (C_Debug_Level >= 3) {
    Serial.print("WiFi.status=");
    Serial.print(WiFi.status());
    Serial.println();
  }
  if (C_Debug_Level >= 2) {
    Serial.print("WiFi connecting to ");
    Serial.print(C_Wifi_SSID);
    Serial.println();
  }
  if (intSetup != 2) return;
  M5.Lcd.fillScreen(BLACK);
  delay(100);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println(" WiFi Setup ");
  M5.Lcd.println(); 
  // MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  String strMAC = HexToString(mac[0]);
  strMAC = String(strMAC + ":" + HexToString(mac[1]));
  strMAC = String(strMAC + ":" + HexToString(mac[2]));
  strMAC = String(strMAC + ":" + HexToString(mac[3]));
  strMAC = String(strMAC + ":" + HexToString(mac[4]));
  strMAC = String(strMAC + ":" + HexToString(mac[5]));
  // text print
  M5.Lcd.print("  MAC ");
  M5.Lcd.println(strMAC);
  M5.Lcd.println(); 
  M5.Lcd.print(" SSID ");
  M5.Lcd.println(C_Wifi_SSID);
  M5.Lcd.println(); 
  M5.update();
}

void printWiFiInfo2() {
  IPAddress ip = WiFi.localIP(); 
  if (C_Debug_Level >= 2) {
    Serial.println();  
  }
  if (C_Debug_Level >= 1) {
    Serial.print("WiFi connected to ");
    Serial.print(C_Wifi_SSID);
    Serial.println();  
    Serial.print("WiFi IPv4: ");
    Serial.println(ip);  
    Serial.print("WiFi Signal strength: ");
    Serial.println(WiFi.RSSI());  
  }
  if (intSetup != 2) return;
  M5.Lcd.print(" IPv4 ");
  M5.Lcd.println(ip);
  M5.Lcd.println(); 
  M5.update();
}

void startOTA() {
  if (intSetup > 2) return;
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}

void printOTAInfo() {
  if (intSetup == 0) return;
  M5.Lcd.print("  OTA Arduino");
  M5.update();
}

void startAtem() {
  // Connecting AtemSwitcher
  IPAddress ipDNS(0, 0, 0, 0);
  printAtemInfo1();
  int intRC = WiFi.hostByName(C_Atem_Name, ipDNS);
  if (intRC == 1) ipAtem = ipDNS;
  printAtemInfo2(intRC);
  AtemSwitcher.begin(ipAtem);
  if (C_Debug_Level >= 4) AtemSwitcher.serialOutput(0x80);
  AtemSwitcher.connect();
  printAtemInfo3();
}

void printAtemInfo1() {
  if (intSetup == 0) return;
  M5.Lcd.fillScreen(BLACK);
  delay(1000);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println(" ATEM Setup");
  M5.Lcd.println();
  M5.Lcd.print(" NAME ");
  M5.Lcd.println(C_Atem_Name);
  M5.Lcd.println();
  M5.update();
}

void printAtemInfo2(int int1) {
  if (C_Debug_Level >= 1) {
    Serial.print("ATEM DNS: ");
    Serial.println(int1); 
    Serial.print("ATEM IPv4: ");
    Serial.print(ipAtem);
    Serial.println();
  }
  if (intSetup == 0) return;
  if (int1 != 1) M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.print("  DNS ");
  M5.Lcd.println(int1);
  M5.Lcd.println();
  M5.Lcd.print(" IPv4 ");
  M5.Lcd.println(ipAtem);
  M5.Lcd.println();
  M5.Lcd.setTextColor(WHITE);
  M5.update();
}

void printAtemInfo3() {
  if (C_Debug_Level >= 1) {
    Serial.println("Atem.begin done");
  }
  if (intSetup == 0) return;
}

void drawLabel(String labelText, unsigned long int labelColor, unsigned long int screenColor, bool ledValue) {
  digitalWrite(LED_PIN, ledValue);
  if (intSetup != 0) return;
  M5.Lcd.fillScreen(screenColor);
  M5.Lcd.setRotation(intOrientation);
  M5.Lcd.setTextColor(labelColor, screenColor);
  drawStringInCenter(labelText, 8);
}

void drawStringInCenter(String input, int font) {
  int datumPrevious = M5.Lcd.getTextDatum();
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString(input, M5.Lcd.width() / 2, M5.Lcd.height() / 2, font);
  M5.Lcd.setTextDatum(datumPrevious);
}

String HexToString(byte Zeichen) {
  String strText = String(Zeichen, HEX);
  if (strText.length() == 1) {
    strText = String("0" + strText);
  }
  strText.toUpperCase();
  return strText;
}

void setM5Orientation() {
  float accX = 0, accY = 0, accZ = 0;
  M5.MPU6886.getAccelData(&accX, &accY, &accZ);
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

// EOF
