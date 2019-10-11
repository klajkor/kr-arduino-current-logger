/**********************************************
** Arduino current meter and logger
** Modules used:
**  - INA219, I2C
**  - DS3231 RTC, I2C
**  - SSD1306 OLED dsiplay, I2C
**  - SD card, SPI
**********************************************/

/* Arduino Nano pinout connections

 ** I2C bus:
 ** SCK - pin A5
 ** SDA - pin A4

 ** SD card on SPI bus:
 ** MOSI - pin D11
 ** MISO - pin D12
 ** CLK - pin D13
 ** CS - pin D4
*/

#include <Arduino.h>
#include <uRTCLib.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <INA219.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* Declarations and initializations */

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
const int OLED_I2C_ADDR = 0x3C; // Address 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Current and voltage sensor
INA219 monitor;

// DS3231 RTC modul
const int RTC_I2C_addr = 0x68;
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
float f_BusVoltage_V;
float f_ShuntCurrent_uA;


// General variables
String TimeStampString = "";
String DateStampString = "";
String VoltString = "";
String CurrentString = "";

// Function definitions
void setup();
void loop();
bool Log_To_SD_card();

//setup()
void setup()
{
  Serial.begin(115200);
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  //display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR);
  Serial.print(F("SSD1306 init...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println(F(" failed!"));
    for(;;); // Don't proceed, loop forever
  }
  Serial.println(F(" OK"));
  Serial.print(F("SD card init..."));

  // see if the card is present and can be initialized:
  if (!SD.begin(SDCARD_CHIP_SELECT)) {
    Serial.println(F(" failed"));
    // don't do anything more:
    while (1);
  }
  Serial.println(F(" OK"));

  Wire.begin();
  //Serial.println(F("Wire.begin done"));
  monitor.begin();
  //Serial.println(F("INA219 begin done"));
  // begin calls:
  // configure() with default values RANGE_32V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT, CONT_SH_BUS
  // calibrate() with default values D_SHUNT=0.1, D_V_BUS_MAX=32, D_V_SHUNT_MAX=0.2, D_I_MAX_EXPECTED=2
  // in order to work directly with ADAFruit's INA219B breakout

  display.clearDisplay();
  display.display();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.display();
  display.println(F("Init done"));
  delay(200);
  display.clearDisplay();
  display.display();

}

void loop()
{
  // get time stamp, convert to a string
  rtc.refresh();
  rtc_second = rtc.second();
  rtc_minute = rtc.minute();
  rtc_hour = rtc.hour();
  rtc_day = rtc.day();
  rtc_month = rtc.month();
  rtc_year = rtc.year();
  
  TimeStampString = F("20");
  TimeStampString += String(rtc_year);
  DateStampString=TimeStampString;
  TimeStampString += F(".");
  //TimeStampString = TimeStampString + String(rtc_year) + F(".");
  //display.print(F("20"));
  //display.print(rtc_year);
  //display.print(F("."));
  if(rtc_month<10) {
    //display.print(F("0"));
    TimeStampString += F("0");
    DateStampString += F("0");
  }
  TimeStampString += String(rtc_month);
  DateStampString += String(rtc_month);
  //display.print(rtc_month);
  TimeStampString += F(".");
  //display.print(F("."));
  if(rtc_day<10) {
    //display.print(F("0"));
    TimeStampString += F("0");
    DateStampString += F("0");
  }
  //display.print(rtc_day);
  TimeStampString += String(rtc_day);
  DateStampString += String(rtc_day);
  //display.print(F(" "));
  TimeStampString += F(" ");
  if(rtc_hour<10) {
    //display.print(F("0"));
    TimeStampString += F("0");
  }
  //display.print(rtc_hour);
  TimeStampString += String(rtc_hour);
  //display.print(F(":"));
  TimeStampString += F(":");
  if(rtc_minute<10) {
    //display.print(F("0"));
    TimeStampString += F("0");
  }
  //display.print(rtc_minute);
  TimeStampString += String(rtc_minute);
  //display.print(F(":"));
  TimeStampString += F(":");
  if(rtc_second<10) {
    //display.print(F("0"));
    TimeStampString += F("0");
  }
  //display.print(rtc_second);
  TimeStampString += String(rtc_second);
  
  // clear display
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  
  //display time stamp
  display.println(TimeStampString);
  Serial.println(TimeStampString);

  //measure voltage and current
  f_ShuntCurrent_uA=monitor.shuntCurrent();
  f_BusVoltage_V=monitor.busVoltage();
  
  //convert to text
  CurrentString=String(f_ShuntCurrent_uA*1000, 4);
  CurrentString+=F(" mA");
  VoltString=String(f_BusVoltage_V,4);
  VoltString+=F(" V");
  
  //display current
  Serial.println(CurrentString);
  display.println(CurrentString);

  //display volt
  Serial.println(VoltString);
  display.println(VoltString);

  display.display();
  
  Serial.print(F("SD log..."));
  if (Log_To_SD_card()) {
    Serial.println(F(" OK"));
  }
  else {
    Serial.println(F(" failed!"));
  }

  delay(5000);

}

bool Log_To_SD_card()
{
  bool = FileOpenSucces = false;
  String logFileName=DateStampString;
  logFileName += F(".txt");
  //open logfile for writing
  File dataFile = SD.open(logFileName, FILE_WRITE);
  
  // if the file is available, write to it:
  if (dataFile) {
    FileOpenSucces=true;
  }
  else {
    FileOpenSucces=false;
  }
  if (FileOpenSuccess) {
    dataFile.print(TimeStampString);
    dataFile.print(F(";"));
    dataFile.print(VoltString);
    dataFile.print(F(";"));
    dataFile.print(CurrentString);
    dataFile.println(F(";"));
    dataFile.close();
  }
  return FileOpenSucces;
}
