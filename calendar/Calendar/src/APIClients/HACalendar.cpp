#include "HACalendar.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"
#include "../../Debug.h"

void HACalendar::addDays(struct tm * timeinfo, struct tm * endtimeinfo, int days) {
  endtimeinfo->tm_year = timeinfo->tm_year;
  endtimeinfo->tm_mon = timeinfo->tm_mon;
  endtimeinfo->tm_mday = timeinfo->tm_mday + days;
  endtimeinfo->tm_hour = timeinfo->tm_hour;
  endtimeinfo->tm_min = timeinfo->tm_min;
  endtimeinfo->tm_sec = timeinfo->tm_sec;
  endtimeinfo->tm_isdst = timeinfo->tm_isdst;
  mktime(endtimeinfo); //Create the correct struct
}

void HACalendar::retrieveCalendarData(struct tm * timeinfo, String calendarName, int numdays) {
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;

    char dateBuf[11];
    strftime(dateBuf,11,"%Y-%m-%d",timeinfo);
    String params="?start="+String(dateBuf)+"T00:00:00.000Z&end=";
    struct tm endtimeinfo;
    addDays(timeinfo,&endtimeinfo,numdays-1); //Extra days is one less than total days
    strftime(dateBuf,11,"%Y-%m-%d",&endtimeinfo);
    params = params + String(dateBuf)+"T23:59:59.000Z";

    String serverName = "http://192.168.178.47:8123/api/calendars/calendar."+calendarName;
    String serverPath = serverName + params;

    Debugln("HTTP GET:");
    Debugln(serverPath);

    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());
    http.addHeader("Authorization", HA_TOKEN);
    http.addHeader("Content-Type", "application/json");

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode>0) {
      Debug("HTTP Response code: ");
      Debugln(httpResponseCode);

      String json = http.getString();
      Debugln("-------");
      Debugln(json);
      Debugln("-------");
      DeserializationError error = deserializeJson(response, json);
      if (error) {
        Debug("deserializeJson() failed: ");
        Debugln(error.f_str());
        return;
      }

      Debugln(response.as<JsonArray>().size());

    }
    else {
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

size_t HACalendar::getEntryCount() {
  return response.as<JsonArray>().size();
}

entry_t HACalendar::getEntry(int index) {
  struct entry_t entry;
  if (index<getEntryCount()) {

    struct tm timeinfo;
    entry.summary = response.as<JsonArray>()[index]["summary"];
    entry.description = response.as<JsonArray>()[index]["description"];
    if (entry.description!=nullptr) {
      entry.urgent = (strchr(entry.description,'!')!=NULL);
      if (strlen(entry.description)>0) {
        entry.eventType = entry.description[0];
      }
      char* p = strchr(entry.description,' ');
      if (p==NULL) {
        entry.eventYear = 0;
      } else {
        entry.eventYear = atoi(p);
      }
    }
    const char* startDateTime = response.as<JsonArray>()[index]["start"]["dateTime"];
    if (startDateTime==nullptr) {
        const char* startDate = response.as<JsonArray>()[index]["start"]["date"];
        strptime(startDate,"%Y-%m-%d",&timeinfo);
        timeinfo.tm_hour=12;
        timeinfo.tm_min=0;
        timeinfo.tm_sec=0;
        mktime(&timeinfo); //Especially for day-of-week
        entry.month = timeinfo.tm_mon;
        entry.mday = timeinfo.tm_mday;
        entry.wday = timeinfo.tm_wday;
        entry.fullDayEvent = true;
    } else {
      strptime(startDateTime,"%Y-%m-%dT%H:%M:%S",&timeinfo);
      mktime(&timeinfo); //Especially for day-of-week
      entry.startHour = timeinfo.tm_hour;
      entry.startMinute = timeinfo.tm_min;
      entry.month = timeinfo.tm_mon;
      entry.mday = timeinfo.tm_mday;
      entry.wday = timeinfo.tm_wday;
      const char* endDateTime = response.as<JsonArray>()[index]["end"]["dateTime"];
      strptime(endDateTime,"%Y-%m-%dT%H:%M:%S",&timeinfo);
      entry.endHour = timeinfo.tm_hour;
      entry.endMinute = timeinfo.tm_min;
      entry.fullDayEvent = false;
    }
  }
  return entry;
}
