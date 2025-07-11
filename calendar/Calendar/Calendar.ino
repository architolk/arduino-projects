#include <time.h>

#include "Debug.h"
#include "src/e-Paper/DEV_Tools.h"
#include "src/e-Paper/EInkDisplay.h"
#include "EPDCalendarCanvas.h"
#include "Config.h"
#include <WiFi.h>
#include "src/APIClients/secrets.h"
#include "src/APIClients/HACalendar.h"
#include "src/APIClients/Weerlive.h"

//See Debug.h for enabling debug mode

//Uncomment this to keep the display state persistant (the display won't be cleared, leaving the last image behind - not recommended if e-Paper is stored)
#define KEEP_DISPLAY_STATE

//Comment if you don't want the ESP32 to go into deep sleep
#define ENABLE_DEEP_SLEEP

//In deep sleep, we can't upload stuff, so we need time before we go into deep sleep
//Minimum 30 seconds, as compiling takes a long time!
//In production, this can be a much lower number
#define SLEEP_INTERVAL 30000

//Timer
#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds

//Displays
EInkDisplay TopDisplay(TOP_PIN_CS, TOP_PIN_DC, TOP_PIN_RST, TOP_PIN_BUSY, TOP_PIN_PWR);
EInkDisplay BottomDisplay(BOTTOM_PIN_CS, BOTTOM_PIN_DC, BOTTOM_PIN_RST, BOTTOM_PIN_BUSY, BOTTOM_PIN_PWR);

//CalendarCanvas
EPDCalendarCanvas canvas(EPD_7IN5B_V2_WIDTH, EPD_7IN5B_V2_HEIGHT);

//Current time stuff
struct tm CurrentTimeInfo;

//Interface to Home Automation Calendar API (per calendar)
HACalendar calFamily;
HACalendar calBirthdays;
//Interface to Weerlive API
Weerlive weather;

void initialize() {
  pinMode(LED_PIN_ESP32, OUTPUT); //Set LED pin to output
  digitalWrite(LED_PIN_ESP32, HIGH); // Set LED pin high, indicating that we have started!

  //Setup pins according to config
  TopDisplay.setupPins();
  BottomDisplay.setupPins();

	// spi
	/*SPI.begin(); //Should probable to SPI.begin(SPI_PIN_SCK, SPI_PIN_MISO, SPI_PIN_MOSI)*/
  SPI.begin(SPI_PIN_SCK, SPI_PIN_MISO, SPI_PIN_MOSI);
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0)); //SPI pins are the default ones...

  Debugln("e-Paper Calendar");
}

int dayIndex(int month, int mday) {
  return month*31 + mday;
}

int readBatteryLevel() {
  Debug("Analog read: ");
  Debugln(analogRead(BATTERY_LEVEL_PIN));
  uint32_t batvolt = analogReadMilliVolts(BATTERY_LEVEL_PIN);
  Debug("Milivolt read: ");
  Debugln(batvolt); //Battery voltage is half the actual voltage
  Debug("Battery status: ");
  Debug((batvolt-1300)/8); //Minimum voltage = 2500mV, Maximum voltage = 4200mV, don't know if we can reach that!
  Debugln("%");
  return (batvolt-1300)/8;
}

void processCalendarEntries(boolean doUrgent) {

  int indexFamily = 0;
  int indexBirthdays = 0;
  entry_t entryFamily;
  entry_t entryBirthdays;

  canvas.setCursor(308,6); //Beginpoint of the calendar. X pos is ignored.
  canvas.displayCalendarResetDayCursor();

  //Get first entries, if available
  if (calBirthdays.getEntryCount()>0) {
    entryBirthdays = calBirthdays.getEntry(0);
  }
  if (calFamily.getEntryCount()>0) {
    entryFamily = calFamily.getEntry(0);
  }
  boolean notFinished = ((calBirthdays.getEntryCount()>0) || (calFamily.getEntryCount()>0));
  while (notFinished) {
    //If family is earlier, show family and go to next event for family
    if ((indexFamily<calFamily.getEntryCount()) && ((dayIndex(entryFamily.month,entryFamily.mday)<dayIndex(entryBirthdays.month,entryBirthdays.mday)) || (indexBirthdays>=calBirthdays.getEntryCount()))) {
      //Show family entry
      if (doUrgent) {
        canvas.displayCalendarEntryUrgent(entryFamily.mday, entryFamily.eventType, entryFamily.urgent);
      } else {
        canvas.displayCalendarEntry(entryFamily.mday, entryFamily.wday, entryFamily.startHour, entryFamily.startMinute, entryFamily.endHour, entryFamily.endMinute, entryFamily.eventType, entryFamily.fullDayEvent, !entryFamily.urgent, String(entryFamily.summary));
      }
      indexFamily++;
      if (indexFamily<calFamily.getEntryCount()) {
        entryFamily = calFamily.getEntry(indexFamily);
      }
    } else {
      //If family is not earlier, show birthday and go to next event for birthdays
      if (indexBirthdays<calBirthdays.getEntryCount()) {
        //Show birthday entry
        String summary = String(entryBirthdays.summary);
        if ((entryBirthdays.eventYear>1900) && (entryBirthdays.eventYear<=(1900+CurrentTimeInfo.tm_year))) {
          summary = summary + " (" + String(1900 + CurrentTimeInfo.tm_year - entryBirthdays.eventYear) + ")";
        }
        if (doUrgent) {
          canvas.displayCalendarEntryUrgent(entryBirthdays.mday, entryBirthdays.eventType,entryBirthdays.urgent);
        } else {
          canvas.displayCalendarEntry(entryBirthdays.mday, entryBirthdays.wday, entryBirthdays.startHour, entryBirthdays.startMinute, entryBirthdays.endHour, entryBirthdays.endMinute, entryBirthdays.eventType, entryBirthdays.fullDayEvent, !entryBirthdays.urgent, summary);
        }
        indexBirthdays++;
        if (indexBirthdays<calBirthdays.getEntryCount()) {
          entryBirthdays = calBirthdays.getEntry(indexBirthdays);
        }

      }
    }
    //Stop if no more entries, or no more space available
    notFinished = (((indexFamily<calFamily.getEntryCount()) || (indexBirthdays<calBirthdays.getEntryCount())) && (canvas.calendarSpaceAvailable()));
  }
}

void updateTopDisplay() {

  Debugln("TopDisplay: e-Paper activated");
  TopDisplay.activate(); //This will power on the display, and set the chip-select to this particular display

  //Initialize screen
  TopDisplay.init();

  Debugln("e-Paper initialized, start drawing");
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

  processCalendarEntries(false); //Don't draw any red (=urgent) items

  canvas.displayStatus(&CurrentTimeInfo,readBatteryLevel());

  // done drawing, so send it off to the display
  TopDisplay.writeCanvas(&canvas, EPD_BLACK_WHITE_LAYER);

  //A bit confusing, but now: 0 = White, 1 = Red
  canvas.fillScreen(0);  // fill backgrund;

  //canvas.setTextColor(1, 0); // Red text, white background
  canvas.displayMonthInfoCurrentDay(&CurrentTimeInfo);

  processCalendarEntries(true); //Draw all red (=urgent) items

  // done drawing, so send it off to the display
  // NB: You should always end with "1" even if no red layer is present (because only at "1" the display is turned on!)
  TopDisplay.writeCanvas(&canvas, EPD_WHITE_RED_LAYER);  // 1 = red layer layer

  Debugln("Display has been updated");

  #ifndef KEEP_DISPLAY_STATE

    Debugln("Wait 5 seconds before display is cleared");
    DEV_Delay_ms(5000);

    Debugln("Clear...(to put the e-Paper in it's original clear screen)");
    TopDisplay.init();
    TopDisplay.clear();
  #endif

  Debugln("Display sleep...");
  TopDisplay.sleep();

  // close PWR
  Debugln("close PWR, Module enters 0 power consumption ...");
  TopDisplay.deactivate();

}

void updateBottomDisplay() {

  Debugln("BottomDisplay: e-Paper activated");
  BottomDisplay.activate(); //This will power on the display, and set the chip-select to this particular display

  //Initialize screen
  BottomDisplay.init();
  //1 = White, 0 = Black
  canvas.fillScreen(1);      // fill background
  canvas.setTextColor(0, 1); // black text, white background

  canvas.setFont(&FreeSansBold16pt7b);
  canvas.drawText(40,25,"5:00");
  canvas.drawText(200,25,"10:00");
  canvas.drawText(350,25,"14:00");
  canvas.drawText(510,25,"18:00");
  canvas.drawText(675,25,"23:00");

  hourWeather_t hourWeather = weather.getHourWeather("05:00",5);
  canvas.displayHourWeather(0, hourWeather.image, hourWeather.temp, hourWeather.windrgr, hourWeather.windbft, hourWeather.neersl);
  hourWeather = weather.getHourWeather("10:00",4);
  canvas.displayHourWeather(1, hourWeather.image, hourWeather.temp, hourWeather.windrgr, hourWeather.windbft, hourWeather.neersl);
  hourWeather = weather.getHourWeather("14:00",4);
  canvas.displayHourWeather(2, hourWeather.image, hourWeather.temp, hourWeather.windrgr, hourWeather.windbft, hourWeather.neersl);
  hourWeather = weather.getHourWeather("18:00",5);
  canvas.displayHourWeather(3, hourWeather.image, hourWeather.temp, hourWeather.windrgr, hourWeather.windbft, hourWeather.neersl);
  hourWeather = weather.getHourWeather("23:00",6);
  canvas.displayHourWeather(4, hourWeather.image, hourWeather.temp, hourWeather.windrgr, hourWeather.windbft, hourWeather.neersl);

  canvas.displayMonthCalendar(&CurrentTimeInfo);

  dayWeather_t dayw;
  for (int i=0; i<weather.getDayCount(); i++) {
    dayw = weather.getDayWeather(i);
    canvas.displayCalendarWeather(&CurrentTimeInfo, i, dayw.image, dayw.min_temp, dayw.max_temp, dayw.neersl_perc_dag);
  }

  int line = 0;
  uint8_t lineDay = 255; //Value that cannot occur
  for (int i=0; i<calBirthdays.getEntryCount(); i++) {
    entry_t entry = calBirthdays.getEntry(i);
    if (entry.mday!=lineDay) {
      line = 0;
      lineDay = entry.mday;
    } else {
      line++;
    }
    int dayIndex = entry.mday - CurrentTimeInfo.tm_mday;
    if (dayIndex<0) {
      dayIndex += canvas.getLastDayOfMonth(CurrentTimeInfo.tm_mon,1900+CurrentTimeInfo.tm_year);
    }
    canvas.displayMonthCalendarEntry(&CurrentTimeInfo, dayIndex, line, String(entry.summary));
  }

  // done drawing, so send it off to the display
  BottomDisplay.writeCanvas(&canvas, EPD_BLACK_WHITE_LAYER);

  //A bit confusing, but now: 0 = White, 1 = Red
  canvas.fillScreen(0);  // fill backgrund;

  canvas.setTextColor(1, 0); // Red text, white background
  canvas.displayMonthCalendarCurrentDay(&CurrentTimeInfo);

  // done drawing, so send it off to the display
  // NB: You should always end with "1" even if no red layer is present (because only at "1" the display is turned on!)
  BottomDisplay.writeCanvas(&canvas, EPD_WHITE_RED_LAYER);  // 1 = red layer layer

  Debugln("Display has been updated");

  #ifndef KEEP_DISPLAY_STATE

    Debugln("Wait 5 seconds before display is cleared");
    DEV_Delay_ms(5000);

    Debugln("Clear...(to put the e-Paper in it's original clear screen)");
    BottomDisplay.init();
    BottomDisplay.clear();
  #endif

  Debugln("Display sleep...");
  BottomDisplay.sleep();

  // close PWR
  Debugln("close PWR, Module enters 0 power consumption ...");
  BottomDisplay.deactivate();

}

void setTimezone(String timezone) {
  Debug("Setting Timezone to ");
  Debugln(timezone.c_str());
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
  Debug("Setting time: ");
  Debugln(asctime(&tm));
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
    setTime(weather.getTimestamp()); //Set time retrieved from the weather API
  } else {
    //Fallback (only works after deep sleep - doesn't work after reset!)
    Debugln("Failed to get timestamp from weather API, use value from RTC");
    /*
    setTime(2025,6,30,21,20,0,0); //11-22-2025 21:20:00 No daylight saving time - but can this not be done automatically?
    */
  }

  if(!getLocalTime(&CurrentTimeInfo)){
    Debugln("Failed to obtain time - set default value");
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
  Debugln("Connecting");
  int count = 0;
  while((count<40) && (WiFi.status() != WL_CONNECTED)) {
    delay(500);
    Debug(".");
    count++;
  }
  if (WiFi.status()==WL_CONNECTED) {
    weather.retrieveWeatherData();
    setupTime();
    calFamily.retrieveCalendarData(&CurrentTimeInfo,"familie",7); //Max one week for family events (top screen)
    calBirthdays.retrieveCalendarData(&CurrentTimeInfo,"verjaardagen",14); //Max two weeks for birthday events (top & bottom screens)
    return true;
  } else {
    return false;
  }
}

time_t calculateSecondsToSleep() {
  struct tm nextDay;
  //Update takes place nightly at 4:30
  nextDay.tm_min = 30;
  nextDay.tm_hour = 4;
  nextDay.tm_sec = 0;
  nextDay.tm_mday = CurrentTimeInfo.tm_mday+1;
  nextDay.tm_mon = CurrentTimeInfo.tm_mon;
  nextDay.tm_year = CurrentTimeInfo.tm_year;
  nextDay.tm_isdst = CurrentTimeInfo.tm_isdst;
  return (difftime(mktime(&nextDay),mktime(&CurrentTimeInfo)));
}

void gotoSleep() {
  Debug("ESP32 Deep sleep after sleep interval: ");
  Debugln(SLEEP_INTERVAL);
  DEV_Delay_ms(SLEEP_INTERVAL);

  time_t sleepTime = calculateSecondsToSleep();
  Debug("Start sleeping for ");
  Debug(sleepTime);
  Debugln(" seconds");

  // Should be depending on the current time, so we always sleep till a particular wake-up time each night
  esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * sleepTime);
  //Enable touch wakeup
  touchSleepWakeUpEnable(TOUCH_PIN_WAKEUP, TOUCH_THRESHOLD);

  Debugln("Start sleeping in one second");
  DEV_Delay_ms(1000);
  esp_deep_sleep_start();
}

void printWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     Debugln("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:     Debugln("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    Debugln("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Debugln("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      Debugln("Wakeup caused by ULP program"); break;
    default:                        Debug("Wakeup was not caused by deep sleep: "); Debugln(wakeup_reason); break;
  }
}

void setup() {

  //Serial - for debugging only
  Serial.begin(115200);

  printWakeupReason();

  if (!canvas.getBuffer()) {
    Debugln("ERROR: Could not allocate buffer for GFXcanvas1");
  } else {
    initialize();
    if (setupWifi()) {
      updateTopDisplay();
      updateBottomDisplay();
    } else {
      Debugln("Wifi not available");
    }
  }

  digitalWrite(LED_PIN_ESP32, LOW); // Set LED to low, indicating that we have finished the programm

#ifdef ENABLE_DEEP_SLEEP
  gotoSleep();
#endif

}

void loop() {
  //Won't get here
}
