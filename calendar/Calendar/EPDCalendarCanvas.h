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
  protected:
    void printDayOfWeek(int wday);
    void printMonth(int mon);
};
#endif
