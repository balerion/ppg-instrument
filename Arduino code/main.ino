#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <microsmooth.h>
#include <SD.h>

#define serial_enable true

// Battery monitoring
#define VBATPIN A9
const float minVoltage = 3.2;
const float maxVoltage = 4.1;

// defines for tacho
#define timeSeconds 10

const float min_rpm = 2000;
const float max_rpm = 8000;
// defines for tacho: Set GPIOs for LED and PIR Motion Sensor
const int tachoPin = 0;
const long updatet = 20;

float rpm_filt = 0;
float ww = 2; // filter weight. larger numbers -> slower filters. 40 is ..s, 1 is no filtering

// defines for tacho: Timer auxiliary variables
unsigned long tt = millis();
unsigned long dt = micros();
unsigned long oldtime = micros();
float rev = 0;
float rpm = 0;

unsigned long lastTrigger = 0;
boolean startTimer = false;

// Defining the type of display used (128x32)
U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Pin definition
const int chargeMeasurePin = A1;
const int batteryMeasurePin = A2;
const int hallSensorPin = A3;

// Defining variables for OLED display
char displayBuffer[20];
String displayString;
unsigned long lastSignalBlink;
unsigned long lastDataRotation;

float batteryVoltage = 0;
float throttle = 0;

void setup()
{
  pinMode(tachoPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(tachoPin), rpm_isr, FALLING);

  // initialize serial:
  if (serial_enable)
  {
    Serial.begin(115200);
  }

  Wire.setClock(400000);
  u8g2.begin();
}

long tt_loop = 0;
long loopUpdateTime = 10;

void loop()
{
  if (serial_enable)
  {
    while (Serial.available() > 0)
    {
      // look for the next valid integer in the incoming serial stream:
      batteryVoltage = Serial.parseInt();
      rev = Serial.parseInt();
      if (Serial.read() == '\n')
      {
        Serial.println(rev, HEX);
      }
    }
    // Serial.println(rpm_filt);
  }
  if (millis() - tt_loop > loopUpdateTime)
  {
    updateTacho();
    readBatteryVoltage();
    updateMainDisplay();
    tt_loop = millis();
  }
}

// Function to calculate and return the thermocouple reading.
float readChtVoltage()
{
  float chtVoltage = 0.0;
  int total = 0;
  int extrabits = 2;
  int nn = pow(2, 2 * extrabits);
  for (int i = 0; i < 16; i++)
  {
    total += analogRead(chtMeasurePin);
  }

  chtVoltage = (refVoltage / 1024.0) * ((float)total / 16.0);

  return chtVoltage;
}

// function for calibrating cht voltage to temperature in °C
float calibratedCht(float voltage)
{
  return (voltage - 1.25) / 0.005;
}

float chtReading = 0;
void readCht()
{
  float chtVoltage = readChtVoltage();
  chtReading = calibratedCht(chtVoltage);
  if (serial_enable)
  {
    Serial.print("CHT: ");
    Serial.println(chtReading);
  }
}

void readBatteryVoltage()
{
  measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  if (serial_enable)
  {
    Serial.print("VBat: ");
    Serial.println(measuredvbat);
  }
}

void rpm_isr()
{
  rev++;
}

void updateTacho()
{
  dt = micros() - oldtime;
  oldtime = micros();
  if (dt > 0)
  {
    rpm = (rev / dt) * 60000000;
    if (!serial_enable)
    {
      rev = 0;
    }
  }
  rpm_filt = constrain((1 / ww) * (rpm) + (1 - (1 / ww)) * rpm_filt, 0, max_rpm);
  if (serial_enable) {
    Serial.print("rpm: ");
    Serial.println(rpm_filt);
  }
}

typedef struct printStruct
{
  float value;
  String suffix;
  String prefix;
  int decimals;
  int bigChars;
  int x;
  int y;
} printStruct;

typedef struct barStruct
{
  float value;
  float max;
  float min;
  int x;
  int y;
} barStruct;

void updateMainDisplay()
{
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

  u8g2.firstPage();
  do
  {
    drawPage();
    drawBatteryLevel();
    drawBar(chtBar);
    drawBar(rpmBar);
  } while (u8g2.nextPage());
}

void drawPage()
{
  printStruct rpmPrint;
  rpmPrint.value = rpm_filt / 1000;
  rpmPrint.suffix = "krpm";
  rpmPrint.prefix = "REVS";
  rpmPrint.decimals = 2;
  rpmPrint.bigChars = 1;
  rpmPrint.x = 0;
  rpmPrint.y = 16;
  smallPrint(rpmPrint);

  printStruct chtPrint;
  chtPrint.value = chtReading;
  chtPrint.suffix = "°C";
  chtPrint.prefix = "CHT";
  chtPrint.decimals = 0;
  chtPrint.bigChars = 3;
  chtPrint.x = 0;
  chtPrint.y = 0;
  smallPrint(chtPrint);
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

void smallPrint(struct printStruct pp)
{
  int first, last;

  // Split up the float value: a number, b decimals.
  first = abs(floor(pp.value));
  last = pp.value * pow(10, pp.decimals) - first * pow(10, pp.decimals);

  // Add leading zeros (2+bigChars-decimals)
  displayString = "";
  for (int i = 0; i < pp.bigChars; i++)
  {
    if (first < pow(10, i))
    {
      displayString = "0" + displayString;
    }
    else
    {
      displayString = (String)first;
    }
  }

  // Display numbers
  displayString.toCharArray(displayBuffer, 10);
  u8g2.setFont(u8g2_font_10x20_tn);
  u8g2.drawStr(pp.x + 55, pp.y + 13, displayBuffer);

  // Display decimal point
  if (pp.decimals > 0)
  {
    displayString = ".";
    displayString.toCharArray(displayBuffer, pp.decimals + 2);
    u8g2.setFont(u8g2_font_10x20_tn);
    u8g2.drawStr(pp.x + 55 + 10 * (pp.bigChars) - 3, pp.y + 13, displayBuffer);
  }

  // Display decimals
  displayString = "";
  for (int i = 0; i < pp.decimals; i++)
  {
    if (first < pow(10, i))
    {
      displayString = "0" + displayString;
    }
    else
    {
      displayString = (String)last;
    }
  }
  // displayString = "." + displayString;

  displayString.toCharArray(displayBuffer, pp.decimals + 2);
  u8g2.setFont(u8g2_font_10x20_tn);
  u8g2.drawStr(pp.x + 55 + 10 * (pp.bigChars) - 3 + 8, pp.y + 13, displayBuffer);

  // Display suffix
  displayString = pp.suffix;
  displayString.toCharArray(displayBuffer, 10);
  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(pp.x + 55 + 10 * (pp.bigChars + pp.decimals) - 3 + (pp.decimals>0 ? 7 : 2) + 3, pp.y + 13, displayBuffer);
}

// Function used to indicate the remotes battery level.
int batteryLevel()
{
  float voltage = measuredvbat;

  if (voltage <= minVoltage)
  {
    return 0;
  }
  else if (voltage >= maxVoltage)
  {
    return 100;
  }
  else
  {
    return (voltage - minVoltage) * 100 / (maxVoltage - minVoltage);
  }
}

void drawBatteryLevel()
{
  int level = batteryLevel();

  // Position on OLED
  int x = 108;
  int y = 4;

  u8g2.drawFrame(x + 2, y, 18, 9);
  u8g2.drawBox(x, y + 2, 2, 5);

  for (int i = 0; i < 5; i++)
  {
    int p = round((100 / 5) * i);
    if (p < level)
    {
      u8g2.drawBox(x + 4 + (3 * i), y + 2, 2, 5);
    }
  }
}

void drawBar(struct barStruct pp) //Draws Battery Level when not being used as Throttle
{
  u8g2.drawHLine(pp.x, pp.y, 52);
  u8g2.drawVLine(pp.x, pp.y, 10);
  u8g2.drawVLine(pp.x + 52, pp.y, 10);
  u8g2.drawHLine(pp.x, pp.y + 10, 5);
  u8g2.drawHLine(pp.x + 52 - 4, pp.y + 10, 5);

  int width = constrain(map(pp.value, pp.min, pp.max, 0, 49), 0, 49);
  for (int i = 0; i < width; i++)
  {
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
