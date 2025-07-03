#ifndef _WEERLIVE_H_
#define _WEERLIVE_H_

#include <ArduinoJson.h>

struct dayWeather_t {
  int mday;
  int mon;
  int min_temp;
  int max_temp;
  char image;
  int neersl_perc_dag;
};

struct hourWeather_t {
  int hour;
  char image;
  int temp;
  int windbft;
  int windrgr;
  int neersl;
};

class Weerlive {
  public:
    void retrieveWeatherData();
    time_t getTimestamp();
    String getForecast();
    dayWeather_t getDayWeather(int index);
    hourWeather_t getHourWeather(const char* timestr);
  protected:
    JsonDocument response;
    size_t getDayCount();
    size_t getHourCount();
    int findImageIndex(const char* target);
    int findHourIndex(const char* target);
};

#endif
