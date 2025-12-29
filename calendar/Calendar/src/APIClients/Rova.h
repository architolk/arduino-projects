#ifndef _ROVA_H_
#define _ROVA_H_

#include <ArduinoJson.h>

struct wentry_t {
  const char* summary;
  uint16_t year;
  uint8_t month;
  uint8_t mday;
  uint8_t wday;
};

class Rova {
  public:
    void retrieveRovaData();
    size_t getEntryCount();
    wentry_t getEntry(int index);
  protected:
    JsonDocument response;
};

#endif
