// EPDCalanderCanvas is a subclass of EPDCanvas
// This subclass contains all the Calendar-specific functions

#ifndef _EPDCALENDARCANVAS_H
#define _EPDCALENDARCANVAS_H

#include <time.h>

#include "src/Graphics/EPDCanvas.h"

#include "src/Fonts/FreeSansBold8pt7b.h"
#include "src/Fonts/FreeSansBold16pt7b.h"
#include "src/Fonts/FreeSans10pt7b.h"
#include "src/Fonts/FreeSans12pt7b.h"
#include "src/Fonts/FreeSans16pt7b.h"
#include "src/Fonts/FreeSans18pt7b.h"
#include "src/Fonts/FreeSans20pt7b.h"
#include "src/Fonts/FreeSans30pt7b.h"
#include "src/Fonts/WeatherIcons26pt7b.h"
#include "src/Fonts/WeatherIcons36pt7b.h"
#include "src/Fonts/WeatherIcons50pt7b.h"

class EPDCalendarCanvas : public EPDCanvas {
  public:
    EPDCalendarCanvas(uint16_t w, uint16_t h) : EPDCanvas(w, h){};
    void displayDateInfo(struct tm * timeinfo);
    void displayMonthInfo(struct tm * timeinfo);
    void displayMonthInfoCurrentDay(struct tm * timeinfo);
    void displayMinMaxTemperature(int minTemp, int maxTemp);
    void displayForecast(const String &forecase);
    void displayWeatherIconRain(char image, int rainperc);
    void displayCalendarEvent(int mday, int wday, int hs, int ms, int he, int me, int type, const String &description);
  protected:
    static constexpr char *DAYSOFWEEK[] = {"ZO","MA","DI","WO","DO","VR","ZA"};
    int dayCursor = 0; //Used to check if a new day has arrived, or still in the current day

    void printDayOfWeek(int wday);
    void printMonth(int mon);
    int getDayColumn(int wday, int mday, int day);
    int getDayRow(int wday, int mday, int day);
    int getLastDayOfMonth(int mon, int year);
};
#endif
