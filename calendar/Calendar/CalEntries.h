#ifndef _CALENTRIES_H
#define _CALENTRIES_H

#include <time.h>
#include <LinkedList.h>

class CalEntry {
  public:
    const char* description;
    uint16_t year;
    uint8_t month;
    uint8_t mday;
    uint8_t wday;
    bool fullDayEvent;
    uint8_t startHour;
    uint8_t startMinute;
    uint8_t endHour;
    uint8_t endMinute;
    char eventType;
    bool urgent;
    uint16_t eventYear; //Like year of birth, year of marriage, etc.

    CalEntry(uint16_t year, uint8_t month, uint8_t mday, uint8_t wday, uint8_t hs, uint8_t ms, uint8_t he, uint8_t me, char type, bool fullDay, bool urgent, const char* description) {
      this->year = year;
      this->month = month;
      this->mday = mday;
      this->wday = wday;
      this->startHour = hs;
      this->startMinute = ms;
      this->endHour = he;
      this->endMinute = me;
      this->eventType = type;
      this->fullDayEvent = fullDay;
      this->urgent = urgent;
      this->description = description;
    }

    bool pastEvent(struct tm * timeinfo) {
      if (this->year < (1900 + timeinfo->tm_year)) return true;
      if (this->year > (1900 + timeinfo->tm_year)) return false;
      if (this->month < timeinfo->tm_mon) return true;
      if (this->month > timeinfo->tm_mon) return false;
      if (this->mday < timeinfo->tm_mday) return true;
      return false;
    }

    void truncateToCurrent(struct tm * timeinfo) {
      this->year = (1900 + timeinfo->tm_year);
      this->month = timeinfo->tm_mon;
      this->mday = timeinfo->tm_mday;
      this->wday = timeinfo->tm_wday;
      this->startHour = 7;
      this->startMinute = 0;
    }
};

class CalEntries {
  public:
    void add(CalEntry* &entry);
    void sort();
    bool first(CalEntry* &entry);
    bool next(CalEntry* &entry);

  protected:
    int current = 0;
    LinkedList<CalEntry*> entries;
};

#endif
