#ifndef _MAGISTER_H_
#define _MAGISTER_H_

#include <time.h>

class Magister {
  public:
    void retrieveMagisterData(struct tm * timeinfo, int _account, char _eventType);
    int getIndex();
    char* getSummary();

    int mday = 0;
    int wday = 0;
    int startHour = 0;
    int startMinute = 0;
    int endHour = 0;
    int endMinute = 0;
    bool hasValue = false;
    char eventType = 'E';

  protected:
    time_t setDay(struct tm * timeinfo);

    int account = 0;
};

#endif
