#include <EEPROM.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "LowPower.h"



// Pin definition
#define VBATPIN A9
#define CHTMEASUREPIN A1
#define RPMPIN 0
#define BUTTONPIN 1
#define RPMPOWER A2
#define CHTPOWER A3


// uncomment this for dev mode
#define DEVMODE 1

// Battery monitoring
const float minVoltage = 3.2;
const float maxVoltage = 4.1;
const float refVoltage = 3.3;

// defines for tacho
const float min_rpm = 2000;
const float max_rpm = 8000;
const long updatet = 20;

float rpm_filt = 0;
float ww = 2;  // filter weight. larger numbers -> slower filters. 40 is ..s, 1
               // is no filtering

// defines for tacho: Timer auxiliary variables
unsigned long tt = millis();
unsigned long dt = micros();
unsigned long oldtime = micros();
float rev = 0;
float rpm = 0;

unsigned long lastTrigger = 0;
boolean startTimer = false;

// Defining the type of display used (128x32)
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);


// Defining variables for OLED display
char displayBuffer[20];
String displayString;
unsigned long lastSignalBlink;
unsigned long lastDataRotation;

float batteryVoltage = 0;
float throttle = 0;

bool wasSleeping = true;
bool awake = true;
float sleepInput = 0;

// time variables
long tt_loop = 0;
long loopUpdateTime = 10;
long tt_button = 0;
long dt_button = 500;
long tt_slowdraw = 0;
long dt_slowdraw = 1000;

unsigned long tt_running = 0;
unsigned long current_runtime = 0;
unsigned long total_runtime = 0;

long EEPROMReadlong(long address) {
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) +
         ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void EEPROMWritelong(int address, long value) {
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

void prepareSleep() {
  u8g2.setPowerSave(1);
  pinMode(RPMPOWER, INPUT);
  pinMode(CHTPOWER, INPUT);
  digitalWrite(RPMPOWER, LOW);
  digitalWrite(CHTPOWER, LOW);
  wasSleeping = false;
  EEPROMWritelong(0, total_runtime);
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
}

void wakeupProc() {
  u8g2.setPowerSave(0);
  u8g2.begin();
  pinMode(RPMPOWER, OUTPUT);
  pinMode(CHTPOWER, OUTPUT);
  digitalWrite(RPMPOWER, HIGH);
  digitalWrite(CHTPOWER, HIGH);
  total_runtime = EEPROMReadlong(0);
}

void setup() {
  pinMode(RPMPIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RPMPIN), rpm_isr, FALLING);

  pinMode(BUTTONPIN, INPUT_PULLUP);

  // initialize serial:
#if defined(DEVMODE)
  Serial.begin(115200);
  Serial.print("Devmode ON");
#endif

  Wire.setClock(400000);
  wakeupProc();
}

void loop() {
  // check sleep status
  if (digitalRead(BUTTONPIN) == LOW) {
    bool switchSleepState = true;
    // stop everything for 1s while checking button is still down
    tt_button = millis();
    do {
      if (digitalRead(BUTTONPIN) == HIGH) switchSleepState = false;
    } while (millis() - tt_button < dt_button);

    // if after 1 second button is still down, switch sleep state
    if (switchSleepState) {
      if (awake) {
        prepareSleep();
        awake = false;
      } else {
        wakeupProc();
        awake = true;
      }
    }
  }

  // waking code
  if (awake) {
#if defined(DEVMODE)
    while (Serial.available() > 0) {
      // look for the next valid integer in the incoming serial stream:
      rev = Serial.parseInt();
      if (Serial.read() == '\n') {
        Serial.println(rev);
        if (rev < 0) {
          total_runtime = -rev;
          Serial.print("resetting total_runtime: ");
          Serial.println(total_runtime);
        }
      }
    }
    // Serial.println(rpm_filt);
#endif
    if (millis() - tt_loop > loopUpdateTime) {
      updateRunningTime();
      updateTacho();
      readCht();
      readBatteryVoltage();
      readCht();
      updateMainDisplay();
      tt_loop = millis();
    }
  }
}

void updateRunningTime() {
  if (rpm_filt > min_rpm) {
    unsigned int dt = millis() - tt_running;
    tt_running = millis();
    current_runtime += dt;
    total_runtime += dt;
  } else {
    current_runtime = 0;
    tt_running = millis();
  }
}

void displayTime() {
  // int x = 89;
  // int y = 22;
  int x = 104;
  int y = 22;
  char buffer[10];  // make this big enough to hold the resulting string
  snprintf(
      buffer, sizeof(buffer), "%02d:%02d:%02d",
      int(current_runtime / 3600000.0),
      int(current_runtime / 60000.0) - int(current_runtime / 3600000.0) * 60,
      int(current_runtime / 1000.0) - int(current_runtime / 60000.0) * 60);

  u8g2.setFont(u8g2_font_profont10_mn);
  u8g2.drawStr(x, y, buffer);

  snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
           int(total_runtime / 3600000.0),
           int(total_runtime / 60000.0) - int(total_runtime / 3600000.0) * 60,
           int(total_runtime / 1000.0) - int(total_runtime / 60000.0) * 60);

  u8g2.setFont(u8g2_font_profont10_mn);
  u8g2.drawStr(x, y + 8, buffer);

  // Serial.print("total runtime: ");
  // Serial.println(total_runtime);
  // buffer[10];  // make this big enough to hold the resulting string
  // snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
  //          int(total_runtime / 3600000.0),
  //          int(total_runtime / 60000.0) - int(total_runtime / 3600000.0) *
  //          60, int(total_runtime / 1000.0) - int(total_runtime / 60000.0) *
  //          60);
  // Serial.println(buffer);

  // time to OLED
  // displayString = "";
  // for (int i = 0; i < pp.decimals; i++) {
  //   if (last < pow(10, i)) {
  //     displayString = "0" + displayString;
  //   } else {
  //     displayString = (String)last;
  //   }
  // }
  // // displayString = "." + displayString;
}

// Function to calculate and return the thermocouple reading.
float readChtVoltage() {
  float chtVoltage = 0.0;
  int total = 0;
  int extrabits = 2;

  int nn = pow(2, 2 * extrabits);
  for (int i = 0; i < 16; i++) {
    total += analogRead(CHTMEASUREPIN);
  }

  chtVoltage = (refVoltage / 1024.0) * ((float)total / 16.0);

  return chtVoltage;
}

// function for calibrating cht voltage to temperature in °C
float calibratedCht(float voltage) { return (voltage - 1.25) / 0.005; }

float chtReading = 0;
void readCht() {
  float chtVoltage = readChtVoltage();
  chtReading = calibratedCht(chtVoltage);
#if defined(DEVMODE)
  Serial.print("CHT: ");
  Serial.println(chtReading);
#endif
}

void readBatteryVoltage() {
  batteryVoltage = analogRead(VBATPIN);
  batteryVoltage *= 2;           // we divided by 2, so multiply back
  batteryVoltage *= refVoltage;  // Multiply by 3.3V, our reference voltage
  batteryVoltage /= 1024.0;      // convert to voltage

#if defined(DEVMODE)
  Serial.print("VBat: ");
  Serial.println(batteryVoltage);
#endif
}

void rpm_isr() { rev++; }

void updateTacho() {
  dt = micros() - oldtime;
  oldtime = micros();
  if (dt > 0) {
    rpm = (rev / dt) * 60000000;
#if !defined(DEVMODE)
    rev = 0;
#endif
  }
  rpm_filt =
      constrain((1 / ww) * (rpm) + (1 - (1 / ww)) * rpm_filt, 0, max_rpm);
#if defined(DEVMODE)
  Serial.print("rpm: ");
  Serial.println(rpm_filt);
#endif
}

typedef struct printStruct {
  float value;
  String suffix;
  String prefix;
  int decimals;
  int bigChars;
  int x;
  int y;
} printStruct;

typedef struct barStruct {
  float value;
  float max;
  float min;
  int x;
  int y;
} barStruct;

void updateMainDisplay() {
  barStruct rpmBar;
  rpmBar.value = rpm_filt / 1000;
  rpmBar.x = 0;
  rpmBar.y = 18;
  rpmBar.max = max_rpm / 1000;
  rpmBar.min = min_rpm / 1000;

  barStruct chtBar;
  chtBar.value = chtReading;
  chtBar.x = 0;
  chtBar.y = 0;
  chtBar.max = 205;
  chtBar.min = 0;

  printStruct rpmPrint;
  rpmPrint.value = rpm_filt / 1000;
  rpmPrint.suffix = "krpm";
  rpmPrint.prefix = "REVS";
  rpmPrint.decimals = 1;
  rpmPrint.bigChars = 1;
  rpmPrint.x = 0;
  rpmPrint.y = 16;

  printStruct chtPrint;
  chtPrint.value = chtReading;
  chtPrint.suffix = "\xB0";
  chtPrint.suffix += "C";
  chtPrint.prefix = "CHT";
  chtPrint.decimals = 0;
  chtPrint.bigChars = 3;
  chtPrint.x = 0;
  chtPrint.y = 0;

  // u8g2.firstPage();
  // do {
  smallPrint(rpmPrint);
  drawBar(chtBar);
  drawBar(rpmBar);
  // if (millis() - tt_slowdraw > dt_slowdraw) {
  // tt_slowdraw = millis();
  smallPrint(chtPrint);
  displayTime();
  drawBatteryLevel();
  u8g2.sendBuffer();
  // }
  // } while (u8g2.nextPage());
}

// void bigPrint(struct printStruct pp)
// {
//   int first, last;

//   // Display prefix (title)
//   displayString = pp.prefix;
//   displayString.toCharArray(displayBuffer, 10);
//   u8g2.setFont(u8g2_font_profont12_tr);
//   u8g2.drawStr(pp.x, pp.y - 1, displayBuffer);

//   // Split up the float value: a number, b decimals.
//   first = abs(floor(pp.value));
//   last = pp.value * pow(10, 2) - first * pow(10, 2);

//   // Add leading zeros (2+bigChars-decimals)
//   if (first < pow(10, 2) && pp.bigChars > 2)
//   {
//     displayString = "0" + (String)first;
//   }
//   else
//   {
//     if (first <= 9 && pp.bigChars > 1)
//     {
//       displayString = "00" + (String)first;
//     }
//     else
//     {
//       displayString = (String)first;
//     }
//   }

//   // Display numbers
//   displayString.toCharArray(displayBuffer, 10);
//   u8g2.setFont(u8g2_font_logisoso22_tn);
//   u8g2.drawStr(pp.x + 55, pp.y + 13, displayBuffer);

//   // Display decimals
//   displayString = ".";

//   if (pp.decimals > 1)
//   {
//     if (last <= 9)
//     {
//       displayString += "0" + (String)last;
//     }
//     else
//     {
//       displayString += (String)last;
//     }
//   }
//   else
//     displayString += (String)last;

//   displayString.toCharArray(displayBuffer, pp.decimals + 2);
//   u8g2.setFont(u8g2_font_profont12_tr);
//   u8g2.drawStr(pp.x + 86, pp.y - 1, displayBuffer);

//   // Display suffix
//   displayString = pp.suffix;
//   displayString.toCharArray(displayBuffer, 10);
//   u8g2.setFont(u8g2_font_profont12_tr);
//   u8g2.drawStr(pp.x + 86 + 2, pp.y + 13, displayBuffer);
// }

void smallPrint(struct printStruct pp) {
  int first, last;

  // Split up the float value: a number, b decimals.
  first = abs(floor(pp.value));
  last = abs(
      floor(pp.value * pow(10, pp.decimals) - first * pow(10, pp.decimals)));

  // Add leading zeros (2+bigChars-decimals)
  displayString = "";
  for (int i = 0; i < pp.bigChars; i++) {
    if (first < pow(10, i)) {
      displayString = "0" + displayString;
    } else {
      displayString = (String)first;
    }
  }

  // Display numbers
  displayString.toCharArray(displayBuffer, 10);
  u8g2.setFont(u8g2_font_10x20_tn);
  u8g2.drawStr(pp.x + 55, pp.y + 13, displayBuffer);

  // Display decimal point
  if (pp.decimals > 0) {
    displayString = ".";
    displayString.toCharArray(displayBuffer, 10);
    u8g2.setFont(u8g2_font_10x20_tn);
    u8g2.drawStr(pp.x + 55 + 10 * (pp.bigChars) - 3, pp.y + 13, displayBuffer);
  }

  // Display decimals
  displayString = "";
  for (int i = 0; i < pp.decimals; i++) {
    if (last < pow(10, i)) {
      displayString = "0" + displayString;
    } else {
      displayString = (String)last;
    }
  }
  // displayString = "." + displayString;

  displayString.toCharArray(displayBuffer, pp.decimals + 2);
  u8g2.setFont(u8g2_font_10x20_tn);
  u8g2.drawStr(pp.x + 55 + 10 * (pp.bigChars) - 3 + 8, pp.y + 13,
               displayBuffer);

  // Display suffix
  displayString = pp.suffix;
  if (displayString.length() < 3) {
    displayString.toCharArray(displayBuffer, 10);
    u8g2.setFont(u8g2_font_profont12_mf);
    u8g2.drawStr(pp.x + 55 + 10 * (pp.bigChars + pp.decimals) - 3 +
                     (pp.decimals > 0 ? 8 : 2) + 2,
                 pp.y + 13, displayBuffer);
  } else {
    displayString.substring(0, 2).toCharArray(displayBuffer, 10);
    u8g2.setFont(u8g2_font_profont10_tr);
    u8g2.drawStr(pp.x + 55 + 10 * (pp.bigChars + pp.decimals) - 3 +
                     (pp.decimals > 0 ? 8 : 2) + 2,
                 pp.y + 13 - 6, displayBuffer);

    displayString.substring(2).toCharArray(displayBuffer, 10);
    u8g2.setFont(u8g2_font_profont10_tr);
    u8g2.drawStr(pp.x + 55 + 10 * (pp.bigChars + pp.decimals) - 3 +
                     (pp.decimals > 0 ? 8 : 2) + 2,
                 pp.y + 13, displayBuffer);
  }
}

// Function used to indicate the remotes battery level.
void drawBatteryLevel() {
  int level = constrain(
      (batteryVoltage - minVoltage) * 100 / (maxVoltage - minVoltage), 0, 100);

  // Position on OLED
  int x = 108;
  int y = 0;

  u8g2.drawFrame(x + 2, y, 18, 9);
  u8g2.drawBox(x, y + 2, 2, 5);

  u8g2.setDrawColor(0);
  u8g2.drawBox(x + 3, y + 1, 16, 7);
  u8g2.setDrawColor(1);

  for (int i = 0; i < 5; i++) {
    int p = round((100 / 5) * i);
    if (p < level) {
      u8g2.drawBox(x + 4 + (3 * i), y + 2, 2, 5);
    }
  }
}

void drawBar(
    struct barStruct pp)  // Draws Battery Level when not being used as Throttle
{
  u8g2.setDrawColor(0);
  u8g2.drawBox(pp.x + 1, pp.y + 1, 50, 8);
  u8g2.setDrawColor(1);
  u8g2.drawHLine(pp.x, pp.y, 52);
  u8g2.drawVLine(pp.x, pp.y, 10);
  u8g2.drawVLine(pp.x + 52, pp.y, 10);
  u8g2.drawHLine(pp.x, pp.y + 10, 5);
  u8g2.drawHLine(pp.x + 52 - 4, pp.y + 10, 5);

  int width = constrain(map(pp.value, pp.min, pp.max, 0, 49), 0, 49);
  for (int i = 0; i < width; i++) {
    u8g2.drawVLine(pp.x + i + 2, pp.y + 2, 7);
  }

  // if (throttle >= 127)
  // {
  //   int width = map(inputValue, 127, 255, 0, 49);

  //   for (int i = 0; i < width; i++)
  //   {
  //     //if( (i % 2) == 0){
  //     u8g2.drawVLine(x + i + 2, y + 2, 7);
  //     //}
  //   }
  // }
  // else
  // {
  //   int width = map(inputValue, 0, 126, 49, 0);
  //   for (int i = 0; i < width; i++)
  //   {
  //     //if( (i % 2) == 0){
  //     u8g2.drawVLine(x + 50 - i, y + 2, 7);
  //     //}
  //   }
  // }
}
