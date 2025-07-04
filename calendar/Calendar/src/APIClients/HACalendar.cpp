#include "HACalendar.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

void HACalendar::addDays(struct tm * timeinfo, struct tm * endtimeinfo, int days) {
  endtimeinfo->tm_year = timeinfo->tm_year;
  endtimeinfo->tm_mon = timeinfo->tm_mon;
  endtimeinfo->tm_mday = timeinfo->tm_mday + days;
  endtimeinfo->tm_hour = timeinfo->tm_hour;
  endtimeinfo->tm_min = timeinfo->tm_min;
  endtimeinfo->tm_sec = timeinfo->tm_sec;
  mktime(endtimeinfo); //Create the correct struct
}

void HACalendar::retrieveCalendarData(struct tm * timeinfo, String calendarName) {
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;

    char dateBuf[11];
    strftime(dateBuf,11,"%Y-%m-%d",timeinfo);
    String params="?start="+String(dateBuf)+"T00:00:00.000Z&end=";
    struct tm endtimeinfo;
    addDays(timeinfo,&endtimeinfo,7);
    strftime(dateBuf,11,"%Y-%m-%d",&endtimeinfo);
    params = params + String(dateBuf)+"T00:00:00.000Z";

    String serverName = "http://192.168.178.47:8123/api/calendars/calendar."+calendarName;
    String serverPath = serverName + params;

    Serial.println("HTTP GET:");
    Serial.println(serverPath);

    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());
    http.addHeader("Authorization", HA_TOKEN);
    http.addHeader("Content-Type", "application/json");

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      String json = http.getString();
      Serial.println("-------");
      Serial.println(json);
      Serial.println("-------");
      DeserializationError error = deserializeJson(response, json);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      Serial.println(response.as<JsonArray>().size());

    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
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
