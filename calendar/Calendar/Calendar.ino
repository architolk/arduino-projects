#include <time.h>

#include "src/e-Paper/DEV_Tools.h"
#include "src/e-Paper/EInkDisplay.h"
#include "EPDCalendarCanvas.h"
#include "Config.h"

//Uncomment this to keep the display state persistant (the display won't be cleared, leaving the last image behind - not recommended if e-Paper is stored)
//#define KEEP_DISPLAY_STATE

//Displays
EInkDisplay TopDisplay(TOP_PIN_CS, TOP_PIN_DC, TOP_PIN_RST, TOP_PIN_BUSY, TOP_PIN_PWR);;
EInkDisplay BottomDisplay(BOTTOM_PIN_CS, BOTTOM_PIN_DC, BOTTOM_PIN_RST, BOTTOM_PIN_BUSY, BOTTOM_PIN_PWR);;

//CalendarCanvas
EPDCalendarCanvas canvas(EPD_7IN5B_V2_WIDTH, EPD_7IN5B_V2_HEIGHT);

//Current time stuff
struct tm CurrentTimeInfo;

void initialize() {
  pinMode(LED_PIN_ESP32, OUTPUT); //Set LED pin to output
  digitalWrite(LED_PIN_ESP32, LOW); // Set LED pin low, indicating that we have started!

  //Setup pins according to config
  TopDisplay.setupPins();
  //BottomDisplay.setupPins();

	// spi
	SPI.begin(); //Should probable to SPI.begin(SPI_PIN_SCK, SPI_PIN_MISO, SPI_PIN_MOSI)
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0)); //SPI pins are the default ones...

  Serial.print("POC ePaper test\r\n");
}

void updateTopDisplay() {

  Serial.print("TopDisplay: e-Paper Init and Clear...\r\n");
  TopDisplay.activate(); //This will power on the display, and set the chip-select to this particular display

  //Initialize screen
  TopDisplay.init();
  //1 = White, 0 = Black
  canvas.fillScreen(1);      // fill background
  canvas.setTextColor(0, 1); // black text, white background

  canvas.displayDateInfo(&CurrentTimeInfo);
  canvas.displayMonthInfo(&CurrentTimeInfo);
  canvas.displayMinMaxTemperature(15.6, 26.2);
  canvas.displayForecast("we verwachten ongelovelijk mooi weer!");
  canvas.displayWeatherIconRain('B', 2.3);

  canvas.fillRect(240,10,5,455,0);

  // done drawing, so send it off to the display
  TopDisplay.writeCanvas(&canvas, EPD_BLACK_WHITE_LAYER);

  //A bit confusing, but now: 0 = White, 1 = Red
  canvas.fillScreen(0);  // fill backgrund;

  canvas.setTextColor(1, 0); // Red text, white background
  canvas.displayMonthInfoCurrentDay(&CurrentTimeInfo);

  // done drawing, so send it off to the display
  // NB: You should always end with "1" even if no red layer is present (because only at "1" the display is turned on!)
  TopDisplay.writeCanvas(&canvas, EPD_WHITE_RED_LAYER);  // 1 = red layer layer

  Serial.print("EPD_Display\r\n");

  #ifndef KEEP_DISPLAY_STATE

    Serial.print("Wait 5 seconds before display is cleared\r\n");
    DEV_Delay_ms(5000);

    Serial.print("Clear...(to put the e-Paper in it's original clear screen)\r\n");
    TopDisplay.init();
    TopDisplay.clear();
  #endif

  Serial.print("Goto Sleep...\r\n");
  TopDisplay.sleep();

  // close PWR
  Serial.print("close PWR, Module enters 0 power consumption ...\r\n");
  TopDisplay.deactivate();

}

void setTimezone(String timezone) {
  Serial.printf("  Setting Timezone to %s\n",timezone.c_str());
  setenv("TZ",timezone.c_str(),1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}

void setTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst) {

  struct tm tm;
  tm.tm_year = yr - 1900;   // Set date
  tm.tm_mon = month-1;
  tm.tm_mday = mday;
  tm.tm_hour = hr;      // Set time
  tm.tm_min = minute;
  tm.tm_sec = sec;
  tm.tm_isdst = isDst;  // 1 or 0
  time_t t = mktime(&tm);
  Serial.printf("Setting time: %s", asctime(&tm));
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
}

void setupTime() {
  //We should get the time from the ntp timeserver first!
  setTimezone("CET-1CEST,M3.5.0,M10.5.0/3"); //Amsterdam
  setTime(2025,6,30,21,20,0,0); //11-22-2025 21:20:00 No daylight saving time - but can this not be done automatically?

  if(!getLocalTime(&CurrentTimeInfo)){
    Serial.println("Failed to obtain time - set default value");
    CurrentTimeInfo.tm_year = 2025 - 1900;   // Set date
    CurrentTimeInfo.tm_mon = 6-1;
    CurrentTimeInfo.tm_mday = 30;
    CurrentTimeInfo.tm_hour = 21;      // Set time
    CurrentTimeInfo.tm_min = 20;
    CurrentTimeInfo.tm_sec = 0;
    CurrentTimeInfo.tm_isdst = 1;  // 1 or 0
  }

}

void setup() {

  //Serial - for debugging only
  Serial.begin(115200);

  if (!canvas.getBuffer()) {
    Serial.print("ERROR: Could not allocate buffer for GFXcanvas1\r\n");
  } else {
    initialize();
    setupTime();
    updateTopDisplay();
  }

  digitalWrite(LED_PIN_ESP32, HIGH); // Set LED to high, indicating that we have finished the programm

}

void loop() {

}
