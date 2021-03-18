
// http://librarymanager/All#M5StickC https://github.com/m5stack/M5StickC
#include <M5StickC.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <WiFiUdp.h>

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
const int C_Debug_Level = 3; // plus Events
//const int C_Debug_Level = 4; // plus AtemSwitcher.serialOutput
const int C_Timeout = 1; // Setup-Timeout in Minuten

// Put your WiFi SSID and Wifi_Pwd here
const char* C_Pgm_Name = "ATEM Tally";
const char* C_Pgm_Version = "v2021-03-18";
const char* C_Wifi_SSID = "?";
const char* C_Wifi_Pwd = "?";

WebServer myWebServer(80);

// Define the Hostname and/or IP address of your ATEM switcher
const char* C_Atem_Name = "atem";
IPAddress ipAtem(192, 168, 10, 240);

ATEMstd myAtemSwitcher;

int intSetupMode = 1;

bool isAutoOrientation = 0;
int intOrientation = 0;
int intOrientationPrevious = 0;
unsigned long intOrientationMillisPrevious = millis();
unsigned long lngButtonAMillis = 0;
unsigned long lngButtonBMillis = 0;
int intUpdateMillis = 0;

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
//  while (intSetupMode != 0) checkCommunication();
  checkSetup();
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
    intCameraNumber = (intCameraNumber % 8) + 1;
    if (C_Debug_Level >= 2) Serial.printf("ATEM next Camera Number: %d\n", intCameraNumber);
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

void checkSetup() {
  if ((intSetupMode == 0) && (WiFi.status() == WL_CONNECTED)) return;
  if (C_Debug_Level >= 2) {
    Serial.printf("\nStarting Setup Mode %d\n", intSetupMode);
  }
  if (intSetupMode == 0) drawLabel(strLcdText, GRAY, BLACK, HIGH);
  if (intSetupMode != 0) initM5();
  if (intSetupMode == 1) {
    stopWiFi();
    printM5Info();
    waitBtnA(1);
    if (intSetupMode == 1) return;
    if (intSetupMode == 0) {
      if (C_Debug_Level >= 1) Serial.println("M5 PowerOff");
      Serial.flush();
      M5.Axp.PowerOff();
    }
  }
  startWiFi();
  if (intSetupMode == 2) {
//    startOTA();
//  printOTAInfo();
    startmyWebServer();
    waitBtnA(2);
    stopmyWebServer();
    if (intSetupMode <= 2) return;
  }
  if (intSetupMode == 3) {
    startAtem();
    waitBtnA(3);
    if (intSetupMode <= 3) return;
  }
  intCameraNumberPrevious = 0;
  if (C_Debug_Level >= 2) {
    Serial.println("ATEM last Camera Number: 0");
  }
  if (intSetupMode == 0) return;
  intCameraNumber = 1;
  if (C_Debug_Level >= 1) {
    Serial.println("ATEM next Camera Number: 1");
  }
  intSetupMode = 0;
}

void restartESP() {
  digitalWrite(LED_PIN, LOW);
  stopWiFi();
  if (C_Debug_Level >= 3) Serial.println("M5 Restart");
  Serial.flush();
  ESP.restart();
}

void waitBtnA(int int1) {
  int intTimeout = C_Timeout + 1;
  int intLedState = HIGH;
  unsigned long lngIntervall = 0;
  unsigned long lngMillis = 0;
//  digitalWrite(LED_PIN, LOW);
  if (C_Debug_Level >= 2) Serial.printf("M5 waiting for BtnA at Setup Mode %d\n", intSetupMode);
  M5.Lcd.setTextColor(WHITE);
  switch (int1) {
    case 0:
      M5.Lcd.print(" Reboot mit ");
      break;
    case 1:
      M5.Lcd.print("  USB Update oder ");
      break;
    case 2:
      M5.Lcd.print(" HTTP Update oder ");
      break;
    case 3:
      M5.Lcd.print("   ");
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
//        if (C_Debug_Level >= 3) Serial.println("M5 GPIO10 LOW=" + String(digitalRead(10)));
        lngIntervall = 10000;
      } else {
        intLedState = HIGH;     
//        if (C_Debug_Level >= 3) Serial.println("M5 GPIO10 HIGH=" + String(digitalRead(10)));
        lngIntervall = 50000;
        intTimeout--;
      }
      if (intTimeout <= 0) {
        intSetupMode--;
        if (C_Debug_Level >= 1) Serial.println("M5 Fallback to Setup " + String(intSetupMode));
        return;
      }
      digitalWrite(LED_PIN, intLedState);
    }
    M5.update();
    if (intSetupMode >= 2) myWebServer.handleClient(); 
    if (intSetupMode >= 3) myAtemSwitcher.runLoop();
  }
  if (C_Debug_Level >= 3) Serial.println("M5.BtnA.wasPressed");
  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.fillScreen(BLACK);
  lngMillis = millis();
  while (M5.BtnA.isPressed() == true) {
    if (millis() - lngMillis >= 500) {
      if (C_Debug_Level >= 3) Serial.println("M5.BtnA.isPressed");
//      if (C_Debug_Level >= 3) Serial.println("M5 GPIO39=" + String(digitalRead(39)));
      return;
    }
    M5.update();
  }
  lngButtonAMillis = 0;
  intSetupMode++;
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
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.print("   M5"); 
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print(" kurz OK lang Setup\n\n"); 
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.print(" BtnB"); 
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print(" Ausrichtung\n\n"); 
  M5.Lcd.println(" WiFi ausgeschaltet");
  M5.Lcd.println();
  M5.update();
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
  if ((intSetupMode == 0) && (WiFi.status() == WL_CONNECTED)) return;
  if ((intSetupMode > 2) && (WiFi.status() == WL_CONNECTED)) return;
  if (intSetupMode > 2) intSetupMode = 2;
  printWiFiInfo1();
  int intLoop = 0;
  int intMillis = 0;
  int intTimeout = C_Timeout + 1;
  unsigned long lngMillis = 0;
  String strHostName = WiFi.macAddress();
  strHostName.replace(":", "");
  strHostName.toLowerCase();
  strHostName = "esp32-" + strHostName.substring(6);
  char chrHostName[13];
  strHostName.toCharArray(chrHostName, 13);
  while (WiFi.status() != WL_CONNECTED) {
    if (intLoop % 20 == 0) {
      digitalWrite(LED_PIN, LOW);
      stopWiFi();
      intTimeout--;
      if (intTimeout <= 0) {
        restartESP();
      }
      digitalWrite(LED_PIN, HIGH);
      delay(1000);
      WiFi.mode(WIFI_STA);
      delay(1000);
      WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
      WiFi.setHostname(chrHostName);  
      WiFi.persistent(false);
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
      if (M5.BtnA.isPressed() && millis() - intMillis >= 500 ) {
        if (C_Debug_Level >= 3) Serial.println("M5.BtnA.isPressed");
        restartESP();
      }
      M5.update();
    }
    intLoop++;
    if (intLoop % 20 == 0) {
      if (C_Debug_Level >= 2) Serial.println();
    }
  }
  printWiFiInfo2();
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
  if (intSetupMode != 2) return;
  M5.Lcd.fillScreen(BLACK);
  delay(100);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println(" WiFi Einstellungen ");
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
  if (intSetupMode != 2) return;
  M5.Lcd.print(" IPv4 ");
  M5.Lcd.println(ip);
  M5.Lcd.println(); 
  M5.update();
}

void startAtem() {
  // Connecting AtemSwitcher
  IPAddress ipDNS(0, 0, 0, 0);
  printAtemInfo1();
  int intRC = WiFi.hostByName(C_Atem_Name, ipDNS);
  if (intRC == 1) ipAtem = ipDNS;
  printAtemInfo2(intRC);
  myAtemSwitcher.begin(ipAtem);
  if (C_Debug_Level >= 4) myAtemSwitcher.serialOutput(0x80);
  myAtemSwitcher.connect();
  printAtemInfo3();
}

void printAtemInfo1() {
  if (intSetupMode == 0) return;
  M5.Lcd.fillScreen(BLACK);
  delay(1000);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println(" ATEM Einstellungen");
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
  if (intSetupMode == 0) return;
  M5.Lcd.setTextColor(GREEN);
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
  if (intSetupMode == 0) return;
}

void drawLabel(String labelText, unsigned long int labelColor, unsigned long int screenColor, bool ledValue) {
  digitalWrite(LED_PIN, ledValue);
  if (intSetupMode != 0) return;
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

String myWebServerIndex = 
"<form method='POST' action='/update' enctype='multipart/form-data'>"
"<br>"
"<input type='file' name='update'>"
"<br><br>"
"<input type='submit' value='Update'>"
"</form>";

void startmyWebServer() {
  myWebServer.on("/", HTTP_GET, []() {
    myWebServer.sendHeader("Connection", "close");
    myWebServer.send(200, "text/html", myWebServerIndex);
  });
  /*handling uploading firmware file */
  myWebServer.on("/update", HTTP_POST, []() {
    myWebServer.sendHeader("Connection", "close");
    myWebServer.send(200, "text/plain", (Update.hasError()) ? "FEHLER" : "OK");
    intSetupMode = 1;
    waitBtnA(0);
    ESP.restart();
  }, []() {
    HTTPUpload& myHTTPUpload = myWebServer.upload();
    if (myHTTPUpload.status == UPLOAD_FILE_START) {
      if (C_Debug_Level >= 1) Serial.printf("Update: %s\n", myHTTPUpload.filename.c_str());
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextColor(YELLOW);
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
        if (C_Debug_Level >= 1) Serial.printf("Update Success: %u Byte\nWait for Reboot...\n", myHTTPUpload.totalSize);
        M5.Lcd.setTextColor(GREEN);
        M5.Lcd.printf("\n\n Ok: %u Byte\n\n", myHTTPUpload.totalSize);
      } else {
        if (C_Debug_Level >= 1) Update.printError(Serial);
        M5.Lcd.setTextColor(RED);
        Update.printError(M5.Lcd);
      }
    }
  });
  myWebServer.begin();
  if (intSetupMode == 0) return;
  if (C_Debug_Level >= 2) {
    Serial.print("HTTP Update Server started");
    Serial.println();
  }
}

void stopmyWebServer() {
  myWebServer.stop(); 
  if (intSetupMode == 0) return;
  if (C_Debug_Level >= 2) {
    Serial.print("HTTP Update Server stopped");
    Serial.println();
  }
}

// EOF
