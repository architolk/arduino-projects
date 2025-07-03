#ifndef _WEERLIVE_H_
#define _WEERLIVE_H_

#include <ArduinoJson.h>

struct dayWeather_t {
  int mday;
  int mon;
  int min_temp;
  int max_temp;
  char image;
  int windbft;
  int windr; //In degrees
  int neersl_perc_dag;
};

class Weerlive {
  public:
    void retrieveWeatherData();
    time_t getTimestamp();
    String getForecast();
    dayWeather_t getDayWeather(int index);
  protected:
    JsonDocument response;
    size_t getDayCount();
    int findImageIndex(const char* target);
};

#endif
