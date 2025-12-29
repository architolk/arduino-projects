#ifndef _MAGISTER_H_
#define _MAGISTER_H_

#include <time.h>

class Magister {
  public:
    void retrieveMagisterData(struct tm * timeinfo, int _account, char _eventType);
    int getIndex();
    char* getSummary();

    uint16_t year = 1900;
    uint8_t month = 0;
    uint8_t mday = 0;
    uint8_t wday = 0;
    uint8_t startHour = 0;
    uint8_t startMinute = 0;
    uint8_t endHour = 0;
    uint8_t endMinute = 0;
    bool hasValue = false;
    char eventType = 'E';

  protected:
    time_t setDay(struct tm * timeinfo);

    int account = 0;
};

#endif
