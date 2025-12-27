#ifndef _HACALENDAR_H_
#define _HACALENDAR_H_

#include <time.h>
#include <ArduinoJson.h>

struct entry_t {
  const char* summary;
  const char* description;
  uint16_t year;
  uint8_t month;
  uint8_t mday;
  uint8_t wday;
  boolean fullDayEvent;
  uint8_t startHour;
  uint8_t startMinute;
  uint8_t endHour;
  uint8_t endMinute;
  char eventType;
  boolean urgent;
  uint16_t eventYear; //Like year of birth, year of marriage, etc.
};

class HACalendar {
  public:
    void retrieveCalendarData(struct tm * timeinfo, String calendarName, int numdays);
    size_t getEntryCount();
    entry_t getEntry(int index);
  protected:
    JsonDocument response;
    void addDays(struct tm * timeinfo, struct tm * endtimeinfo, int days);
};

#endif
