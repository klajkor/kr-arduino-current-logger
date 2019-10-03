/**********************************************
** Arduino current meter and logger
** Modueles used:
**  - INA219, I2C
**  - RTC, I2C
**  - SSD1306 OLED dsiplay, I2C
**  - SD card, SPI
**********************************************/

/*

 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 
 */

#include <SPI.h>
#include <SD.h>

#include "Arduino.h"
#include "uRTCLib.h"
#include <Wire.h>
#include <INA219.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* Declarations and initializations */

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Current and voltage sensor
INA219 monitor;

// RTC modul
#define RTC_I2C_addr 0x68
uRTCLib rtc(RTC_I2C_addr);

// SD card modul
#define SDCARD_CHIP_SELECT 4


/* Global variables */

// RTC global variables
uint8_t rtc_second = 0;
uint8_t rtc_minute = 0;
uint8_t rtc_hour = 0;
uint8_t rtc_day = 0;
uint8_t rtc_month = 0;
uint8_t rtc_year = 0;

// Current sensor variables
float f_BusVoltage;


// General variables
String TimeStampString = "";
String VoltString = "";
String CurrentString = "";

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  monitor.begin();
  // begin calls:
  // configure() with default values RANGE_32V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT, CONT_SH_BUS
  // calibrate() with default values D_SHUNT=0.1, D_V_BUS_MAX=32, D_V_SHUNT_MAX=0.2, D_I_MAX_EXPECTED=2
  // in order to work directly with ADAFruit's INA219B breakout

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(SDCARD_CHIP_SELECT)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
  display.clearDisplay();
  display.display();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("SD card init done"));
  display.display();
  delay(200);
  display.clearDisplay();
  display.display();
  
}

void loop()
{
  rtc.refresh();
  rtc_second = rtc.second();
  rtc_minute = rtc.minute();
  rtc_hour = rtc.hour();
  rtc_day = rtc.day();
  rtc_month = rtc.month();
  rtc_year = rtc.year();

  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  
  Serial.println("******************");
  
  Serial.print("raw shunt voltage: ");
  Serial.println(monitor.shuntVoltageRaw());
  
  Serial.print("raw bus voltage:   ");
  Serial.println(monitor.busVoltageRaw());
  
  Serial.println("--");
  
  Serial.print("shunt voltage: ");
  Serial.print(monitor.shuntVoltage() * 1000, 4);
  Serial.println(" mV");
  
  Serial.print("shunt current: ");
  Serial.print(monitor.shuntCurrent() * 1000, 4);
  Serial.println(" mA");

  TimeStampString = F("20");
  TimeStampString += String(rtc_year);
  display.print(F("20"));
  display.print(rtc_year);
  display.print(F("."));
  if(rtc_month<10) {
    display.print(F("0"));
  }
  display.print(rtc_month);
  display.print(F("."));
  if(rtc_day<10) {
    display.print(F("0"));
  }
  display.print(rtc_day);
  display.print(F(" "));
  if(rtc_hour<10) {
    display.print(F("0"));
  }
  display.print(rtc_hour);
  display.print(F(":"));
  if(rtc_minute<10) {
    display.print(F("0"));
  }
  display.print(rtc_minute);
  display.print(F(":"));
  if(rtc_second<10) {
    display.print(F("0"));
  }
  display.print(rtc_second);
  display.println(F(" "));
  //display.print(F("Current: "));
  display.print(monitor.shuntCurrent()*1000, 4);
  display.println(F(" mA"));

  Serial.print("bus voltage:   ");
  Serial.print(monitor.busVoltage(), 4);
  Serial.println(" V");

  //display.print(F("Bus V: "));
  display.print(monitor.busVoltage(), 4);
  display.println(F(" V"));

  Serial.print("bus power:     ");
  Serial.print(monitor.busPower() * 1000, 4);
  Serial.println(" mW");
  
  Serial.println(" ");
  Serial.println(" ");

  display.display();
  
  delay(5000);

}
