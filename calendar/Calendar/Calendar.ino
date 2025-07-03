#include <time.h>

#include "src/e-Paper/DEV_Tools.h"
#include "src/e-Paper/EInkDisplay.h"
#include "EPDCalendarCanvas.h"
#include "Config.h"
#include <WiFi.h>
#include "src/APIClients/secrets.h"
#include "src/APIClients/HACalendar.h"
#include "src/APIClients/Weerlive.h"

//Uncomment this to keep the display state persistant (the display won't be cleared, leaving the last image behind - not recommended if e-Paper is stored)
//#define KEEP_DISPLAY_STATE

//Displays
EInkDisplay TopDisplay(TOP_PIN_CS, TOP_PIN_DC, TOP_PIN_RST, TOP_PIN_BUSY, TOP_PIN_PWR);;
EInkDisplay BottomDisplay(BOTTOM_PIN_CS, BOTTOM_PIN_DC, BOTTOM_PIN_RST, BOTTOM_PIN_BUSY, BOTTOM_PIN_PWR);;

//CalendarCanvas
EPDCalendarCanvas canvas(EPD_7IN5B_V2_WIDTH, EPD_7IN5B_V2_HEIGHT);

//Current time stuff
struct tm CurrentTimeInfo;

//Interface to Home Automation Calendar API
HACalendar cal;
//Interface to Weerlive API
Weerlive weather;

void initialize() {
  pinMode(LED_PIN_ESP32, OUTPUT); //Set LED pin to output
  digitalWrite(LED_PIN_ESP32, LOW); // Set LED pin low, indicating that we have started!

  //Setup pins according to config
  TopDisplay.setupPins();
  BottomDisplay.setupPins();

	// spi
	SPI.begin(); //Should probable to SPI.begin(SPI_PIN_SCK, SPI_PIN_MISO, SPI_PIN_MOSI)
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0)); //SPI pins are the default ones...

  Serial.println("e-Paper Calendar");
}

void updateTopDisplay() {

  Serial.println("TopDisplay: e-Paper activated");
  TopDisplay.activate(); //This will power on the display, and set the chip-select to this particular display

  //Initialize screen
  TopDisplay.init();
  //1 = White, 0 = Black
  canvas.fillScreen(1);      // fill background
  canvas.setTextColor(0, 1); // black text, white background

  dayWeather_t dayw = weather.getDayWeather(0);

  canvas.displayDateInfo(&CurrentTimeInfo);
  canvas.displayMonthInfo(&CurrentTimeInfo);
  canvas.displayMinMaxTemperature(dayw.min_temp, dayw.max_temp);
  canvas.displayForecast(weather.getForecast());
  canvas.displayWeatherIconRain(dayw.image, dayw.neersl_perc_dag);

  canvas.fillRect(240,10,5,455,0);

  canvas.setCursor(308,6); //Beginpoint of the calendar. X pos is ignored.
  for (int i=0; i<cal.getEntryCount(); i++) {
    entry_t entry = cal.getEntry(0);
    canvas.displayCalendarEvent(entry.mday, entry.wday, entry.startHour, entry.startMinute, entry.endHour, entry.endMinute, 1, String(entry.summary));
  }
  canvas.displayCalendarEvent(CurrentTimeInfo.tm_mday, CurrentTimeInfo.tm_wday, 8, 20, 14, 15, 2, "school");
  canvas.displayCalendarEvent(CurrentTimeInfo.tm_mday, CurrentTimeInfo.tm_wday, 8, 00, 17, 00, 2, "kantoor");
  canvas.displayCalendarEvent(CurrentTimeInfo.tm_mday+1, CurrentTimeInfo.tm_wday+1, 8, 00, 17, 00, 2, "schoolfotograaf!");

  // done drawing, so send it off to the display
  TopDisplay.writeCanvas(&canvas, EPD_BLACK_WHITE_LAYER);

  //A bit confusing, but now: 0 = White, 1 = Red
  canvas.fillScreen(0);  // fill backgrund;

  //canvas.setTextColor(1, 0); // Red text, white background
  canvas.displayMonthInfoCurrentDay(&CurrentTimeInfo);

  // done drawing, so send it off to the display
  // NB: You should always end with "1" even if no red layer is present (because only at "1" the display is turned on!)
  TopDisplay.writeCanvas(&canvas, EPD_WHITE_RED_LAYER);  // 1 = red layer layer

  Serial.println("Display has been updated");

  #ifndef KEEP_DISPLAY_STATE

    Serial.println("Wait 5 seconds before display is cleared");
    DEV_Delay_ms(5000);

    Serial.println("Clear...(to put the e-Paper in it's original clear screen)");
    TopDisplay.init();
    TopDisplay.clear();
  #endif

  Serial.println("Goto Sleep...");
  TopDisplay.sleep();

  // close PWR
  Serial.println("close PWR, Module enters 0 power consumption ...");
  TopDisplay.deactivate();

}

void updateBottomDisplay() {

  Serial.println("BottomDisplay: e-Paper activated");
  BottomDisplay.activate(); //This will power on the display, and set the chip-select to this particular display

  //Initialize screen
  BottomDisplay.init();
  //1 = White, 0 = Black
  canvas.fillScreen(1);      // fill background
  canvas.setTextColor(0, 1); // black text, white background

  canvas.setFont(&FreeSansBold16pt7b);
  canvas.drawText(40,25,"3:00");
  canvas.drawText(200,25,"7:00");
  canvas.drawText(350,25,"12:00");
  canvas.drawText(510,25,"18:00");
  canvas.drawText(675,25,"23:00");

  //Incorrect: index is not what we want, but the actual hours!
  hourWeather_t hourWeather = weather.getHourWeather("03:00");
  canvas.displayHourWeather(0, hourWeather.image, hourWeather.temp, hourWeather.windrgr, hourWeather.windbft, hourWeather.neersl);
  hourWeather = weather.getHourWeather("07:00");
  canvas.displayHourWeather(1, hourWeather.image, hourWeather.temp, hourWeather.windrgr, hourWeather.windbft, hourWeather.neersl);
  hourWeather = weather.getHourWeather("12:00");
  canvas.displayHourWeather(2, hourWeather.image, hourWeather.temp, hourWeather.windrgr, hourWeather.windbft, hourWeather.neersl);
  hourWeather = weather.getHourWeather("18:00");
  canvas.displayHourWeather(3, hourWeather.image, hourWeather.temp, hourWeather.windrgr, hourWeather.windbft, hourWeather.neersl);
  hourWeather = weather.getHourWeather("23:00");
  canvas.displayHourWeather(4, hourWeather.image, hourWeather.temp, hourWeather.windrgr, hourWeather.windbft, hourWeather.neersl);

  canvas.displayMonthCalendar();

  // done drawing, so send it off to the display
  BottomDisplay.writeCanvas(&canvas, EPD_BLACK_WHITE_LAYER);

  //A bit confusing, but now: 0 = White, 1 = Red
  canvas.fillScreen(0);  // fill backgrund;

  canvas.setTextColor(1, 0); // Red text, white background

  //CODE FOR DRAWING RED SHOULD BE HERE

  // done drawing, so send it off to the display
  // NB: You should always end with "1" even if no red layer is present (because only at "1" the display is turned on!)
  BottomDisplay.writeCanvas(&canvas, EPD_WHITE_RED_LAYER);  // 1 = red layer layer

  Serial.println("Display has been updated");

  #ifndef KEEP_DISPLAY_STATE

    Serial.println("Wait 5 seconds before display is cleared");
    DEV_Delay_ms(5000);

    Serial.println("Clear...(to put the e-Paper in it's original clear screen)");
    BottomDisplay.init();
    BottomDisplay.clear();
  #endif

  Serial.println("Goto Sleep...");
  BottomDisplay.sleep();

  // close PWR
  Serial.println("close PWR, Module enters 0 power consumption ...");
  BottomDisplay.deactivate();

}

void setTimezone(String timezone) {
  Serial.printf("  Setting Timezone to %s\n",timezone.c_str());
  setenv("TZ",timezone.c_str(),1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}

//Setting time using discrete entries
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
  setTime(t);
}

//Setting time using epoch time
void setTime(time_t epoch) {
  struct timeval now = { .tv_sec = epoch };
  settimeofday(&now, NULL);
}

void setupTime() {
  //We should get the time from the ntp timeserver first!
  setTimezone("CET-1CEST,M3.5.0,M10.5.0/3"); //Amsterdam
  time_t epoch = weather.getTimestamp();
  if (epoch > 1751095685) {//It needs to be a timestamp after the compiletime of this routine!
    setTime(weather.getTimestamp());
  } else {
    //Fallback
    Serial.println("Failed to get timestamp from weather API, use default value");
    setTime(2025,6,30,21,20,0,0); //11-22-2025 21:20:00 No daylight saving time - but can this not be done automatically?
  }

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

boolean setupWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting");
  int count = 0;
  while((count<40) && (WiFi.status() != WL_CONNECTED)) {
    delay(500);
    Serial.print(".");
    count++;
  }
  if (WiFi.status()==WL_CONNECTED) {
    cal.retrieveCalendarData(&CurrentTimeInfo);
    weather.retrieveWeatherData();
    return true;
  } else {
    return false;
  }
}

void setup() {

  //Serial - for debugging only
  Serial.begin(115200);

  if (!canvas.getBuffer()) {
    Serial.println("ERROR: Could not allocate buffer for GFXcanvas1");
  } else {
    initialize();
    if (setupWifi()) {
      setupTime();
      //updateTopDisplay();
      updateBottomDisplay();
    } else {
      Serial.println("Wifi not available");
    }
  }

  digitalWrite(LED_PIN_ESP32, HIGH); // Set LED to high, indicating that we have finished the programm

}

void loop() {

}
