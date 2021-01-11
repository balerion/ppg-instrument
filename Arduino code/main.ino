#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <microsmooth.h>

// defines for tacho
#define timeSeconds 10
// defines for tacho: Set GPIOs for LED and PIR Motion Sensor
const int led = 12;
const int tachoPin = 0;
const long updatet = 100;

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

// Battery monitoring
const float minVoltage = 3.2;
const float maxVoltage = 4.1;
const float refVoltage = 5.1; // Set to 4.5V if you are testing connected to USB, otherwise 5V (or the supply voltage)

// Defining variables for OLED display
char displayBuffer[20];
String displayString;
short displayData = 0;
unsigned long lastSignalBlink;
unsigned long lastDataRotation;

float batteryVoltage = 0;
float throttle = 0;

void setup() {
  pinMode(tachoPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(tachoPin), isr, FALLING);

  // initialize serial:
  Serial.begin(115200);
  u8g2.begin();
}

long looptime = 0;
long loopUpdateTime = 50;
void loop() {
  while (Serial.available() > 0) {

    // look for the next valid integer in the incoming serial stream:
    batteryVoltage = Serial.parseInt();
    throttle = Serial.parseInt();

    if (Serial.read() == '\n') {
      Serial.println(batteryVoltage, HEX);
    }
  }
  if (millis()-looptime > loopUpdateTime) {
    updateTacho();
    updateMainDisplay();
    looptime = millis();
  }

}


void isr() {
  rev++;
}

float filtered_value = 0;
float ww = 40; // this is the filter weight. The larger the number, the ... the filter. 40 corresponds to ..s

void updateTacho() {
  dt = micros() - oldtime;
  if (dt > 0) {
    rpm = (rev / dt) * 60000000;
  }
  oldtime = micros();
  rev = 0;
  filtered_value = ww * (rpm) + (1 - ww) * filtered_value;

  if (tt-millis()>updatet) {
    tt=millis();
    Serial.println(dt);
  }
  
}


// Function to calculate and return the remotes battery voltage.
//float batteryVoltage()
//{
//  float batteryVoltage = 0.0;
//  int total = 0;
//
//  for (int i = 0; i < 16; i++)
//  {
//    total += analogRead(batteryMeasurePin);
//  }
//
//  batteryVoltage = (refVoltage / 1024.0) * ((float)total / 16.0);
//
//  return batteryVoltage;
//}

void updateMainDisplay() {

  u8g2.firstPage();
  do {
    drawPage();
    drawBatteryLevel();
    drawThrottle();
  } while (u8g2.nextPage());


}

void drawPage()
{
  int decimals;
  float value;
  String suffix;
  String prefix;

  int first, last;

  int x = 0;
  int y = 16;

  value = throttle;
  suffix = "rpm";
  prefix = "REVS";
  decimals = 1;

// Display prefix (title)
  displayString = prefix;
  displayString.toCharArray(displayBuffer, 10);
  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(x, y - 1, displayBuffer);

  // Split up the float value: a number, b decimals.
  first = abs(floor(value));
  last = value * pow(10, 2) - first * pow(10, 2);

  // Add leading zero
  if (first <= 9)
  {
    displayString = "0" + (String)first;
  }
  else
  {
    displayString = (String)first;
  }



  // Display numbers
  displayString.toCharArray(displayBuffer, 10);
  u8g2.setFont(u8g2_font_logisoso22_tn);
  u8g2.drawStr(x + 55, y + 13, displayBuffer);

  // Display decimals
  if (displayData != 4) displayString = "."; //Disabled the . for the total_km
  else displayString = "";

  if (decimals > 1) {
    if (last <= 9)
    {
      displayString += "0" + (String)last;
    }
    else
    {
      displayString += (String)last;
    }
  }
  else displayString += (String)last;



  displayString.toCharArray(displayBuffer, decimals + 2);
  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(x + 86, y - 1, displayBuffer);

  // Display suffix
  displayString = suffix;
  displayString.toCharArray(displayBuffer, 10);
  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(x + 86 + 2, y + 13, displayBuffer);
}


// Function used to indicate the remotes battery level.
int batteryLevel() {
  float voltage = batteryVoltage;

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
  int level = batteryVoltage;

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


void drawThrottle() //Draws Battery Level when not being used as Throttle
{
  int inputValue = throttle;
  int percent;

  int x = 0;
  int y = 18;

  // Draw throttle
  u8g2.drawHLine(x, y, 52);
  u8g2.drawVLine(x, y, 10);
  u8g2.drawVLine(x + 52, y, 10);
  u8g2.drawHLine(x, y + 10, 5);
  u8g2.drawHLine(x + 52 - 4, y + 10, 5);

  if (throttle >= 127)
  {
    int width = map(inputValue, 127, 255, 0, 49);

    for (int i = 0; i < width; i++)
    {
      //if( (i % 2) == 0){
      u8g2.drawVLine(x + i + 2, y + 2, 7);
      //}
    }
  }
  else
  {
    int width = map(inputValue, 0, 126, 49, 0);
    for (int i = 0; i < width; i++)
    {
      //if( (i % 2) == 0){
      u8g2.drawVLine(x + 50 - i, y + 2, 7);
      //}
    }
  }
}
