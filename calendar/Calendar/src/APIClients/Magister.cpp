#include "Magister.h"
#include "secrets.h"
#include "../../Debug.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <uICAL.h>

#define START_HOUR 4

time_t Magister::setDay(struct tm * timeinfo) {
  struct tm settimeinfo;
  settimeinfo.tm_year = timeinfo->tm_year;
  settimeinfo.tm_mon = timeinfo->tm_mon;
  settimeinfo.tm_mday = timeinfo->tm_mday;
  settimeinfo.tm_hour = START_HOUR;
  settimeinfo.tm_min = 0;
  settimeinfo.tm_sec = timeinfo->tm_sec;
  settimeinfo.tm_isdst = timeinfo->tm_isdst;
  return mktime(&settimeinfo); //Create the correct struct
}

int Magister::getIndex() {
  if (hasValue) {
    return startHour*60 + startMinute;
  } else {
    return 1440;
  }
}

char* Magister::getSummary() {
  if (account==0) {
    return SUMMARY_0;
  } else {
    return SUMMARY_1;
  }
}

void Magister::retrieveMagisterData(struct tm * timeinfo, int _account, char _eventType) {
  mday = timeinfo->tm_mday;
  wday = timeinfo->tm_wday;
  account = _account;
  eventType = _eventType;

  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;

    String serverPath = "https://calendar.magister.net/api/icalendar/feeds/";
    if (account==0) {
      serverPath = serverPath + FEED_0;
    } else {
      serverPath = serverPath + FEED_1;
    }

    Debugln("HTTP GET:");
    Debugln(serverPath);

    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode>0) {
      Debug("HTTP Response code: ");
      Debugln(httpResponseCode);

      uICAL::Calendar_ptr cal = nullptr;
      try {
        uICAL::istream_Stream istm(http.getStream());
        cal = uICAL::Calendar::load(istm);

        time_t epoch = setDay(timeinfo);
        struct tm * utctime = gmtime(&epoch);
        Debug("Time offset: ");
        int offset = START_HOUR - utctime->tm_hour;
        Debugln(offset);
        uICAL::DateTime calBegin(epoch);
        uICAL::DateTime calEnd(epoch + 64800); // = 60*60*18, 18 hours from start hour (= between 4 and 22)
        uICAL::CalendarIter_ptr calIt = uICAL::new_ptr<uICAL::CalendarIter>(cal, calBegin, calEnd);

        int startMinutes = 1440;
        int endMinutes = 0;
        while (calIt->next()) {
            uICAL::CalendarEntry_ptr entry = calIt->current();
            /*
            Debug("Event: ");
            Debug(offset + entry->start().datestamp().hour);
            Debug(":");
            Debug(entry->start().datestamp().minute);
            Debug(" - ");
            Debug(offset + entry->end().datestamp().hour);
            Debug(":");
            Debug(entry->end().datestamp().minute);
            Debug(" > ");
            Debugln(entry->summary().c_str());
            */
            if (entry->start().datestamp().hour!=0) { //All day events have hour==0, so skip those
              int entryStartMinutes = (offset + entry->start().datestamp().hour)*60 + entry->start().datestamp().minute;
              int entryEndMinutes = (offset + entry->end().datestamp().hour)*60 + entry->end().datestamp().minute;
              if (entryStartMinutes < startMinutes) {
                startMinutes = entryStartMinutes;
              }
              if (entryEndMinutes > endMinutes) {
                endMinutes = entryEndMinutes;
              }
            }
        }
        if ((startMinutes!=1440) && (endMinutes!=0)) {
          hasValue = true;
          startHour = startMinutes / 60;
          startMinute = startMinutes % 60;
          endHour = endMinutes / 60;
          endMinute = endMinutes % 60;
        }

      } catch (uICAL::Error ex) {
        Debug("Failed loading calendar: ");
        Debugln(ex.message.c_str());
      }
    } else {
      Debug("Error code: ");
      Debugln(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Debugln("WiFi Disconnected");
  }
}
