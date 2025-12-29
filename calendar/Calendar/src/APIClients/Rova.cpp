#include "Rova.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"
#include "../../Debug.h"

void Rova::retrieveRovaData() {

  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;

    String serverPath = "https://www.rova.nl/api/waste-calendar/upcoming?postalcode=";
    serverPath = serverPath+POSTALCODE+"&houseNumber="+HOUSENUMBER+"&addition=&take=5";

    Debugln("HTTP GET:");
    Debugln(serverPath);

    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

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

size_t Rova::getEntryCount() {
  return response.as<JsonArray>().size();
}

wentry_t Rova::getEntry(int index) {
  struct wentry_t entry;
  if (index<getEntryCount()) {
    entry.summary = response.as<JsonArray>()[index]["wasteType"]["title"];
    const char* startDate = response.as<JsonArray>()[index]["date"];
    struct tm timeinfo;
    strptime(startDate,"%Y-%m-%d",&timeinfo);
    timeinfo.tm_hour=12;
    timeinfo.tm_min=0;
    timeinfo.tm_sec=0;
    mktime(&timeinfo); //Especially for day-of-week
    entry.year = 1900 + timeinfo.tm_year;
    entry.month = timeinfo.tm_mon;
    entry.mday = timeinfo.tm_mday;
    entry.wday = timeinfo.tm_wday;
  }
  return entry;
}
