#include "Weerlive.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

const char* images[] = {"zonnig","bliksem","regen","buien","hagel","mist","sneeuw","bewolkt","lichtbewolkt","halfbewolkt","halfbewolkt_regen","zwaarbewolkt","nachtmist","helderenacht","nachtbewolkt"};

int Weerlive::findImageIndex(const char* target) {
  for (int i=0; i<15; i++) {
    if (strcmp(target, images[i]) == 0) {
      return i;
    }
  }
  return -1;
}

int Weerlive::findHourIndex(const char* target) {
  for (int i=0; i<getHourCount(); i++) {
    const char* datetime = response["uur_verw"].as<JsonArray>()[i]["uur"];
    if (strstr(datetime,target) != NULL) {
      return i;
    }
  }
  return -1;
}

void Weerlive::retrieveWeatherData() {
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;

    String params="?key=";
    params+= WL_APIKEY;
    params+= "&locatie=";
    params+= WL_LOCATION;

    String serverName = "https://weerlive.nl/api/weerlive_api_v2.php";
    String serverPath = serverName + params;

    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

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

time_t Weerlive::getTimestamp() {
  return response["liveweer"].as<JsonArray>()[0]["timestamp"];
}

String Weerlive::getForecast() {
  const char* verw = response["liveweer"].as<JsonArray>()[0]["verw"];
  return String(verw);
}

dayWeather_t Weerlive::getDayWeather(int index) {
  dayWeather_t dayWeather;
  if (index<getDayCount()) {
    dayWeather.min_temp = response["wk_verw"].as<JsonArray>()[index]["min_temp"];
    dayWeather.max_temp = response["wk_verw"].as<JsonArray>()[index]["max_temp"];
    dayWeather.neersl_perc_dag = response["wk_verw"].as<JsonArray>()[index]["neersl_perc_dag"];
    dayWeather.image = char(65+findImageIndex(response["wk_verw"].as<JsonArray>()[index]["image"]));
  } else {
    //Default dayWeather
    dayWeather.min_temp = 0;
    dayWeather.max_temp = 0;
    dayWeather.neersl_perc_dag = 0;
    dayWeather.image = char(64);
  }
  return dayWeather;
}

size_t Weerlive::getDayCount() {
  return response["wk_verw"].as<JsonArray>().size();
}

hourWeather_t Weerlive::getHourWeather(const char* timestr) {
  hourWeather_t hourWeather;
  int index = findHourIndex(timestr);
  if (index>=0) {
    hourWeather.temp = response["uur_verw"].as<JsonArray>()[index]["temp"];
    hourWeather.windbft = response["uur_verw"].as<JsonArray>()[index]["windbft"];
    hourWeather.windrgr = response["uur_verw"].as<JsonArray>()[index]["windrgr"];
    hourWeather.neersl = response["uur_verw"].as<JsonArray>()[index]["neersl"];
    hourWeather.image = char(65+findImageIndex(response["uur_verw"].as<JsonArray>()[index]["image"]));
  } else {
    //Default hourWeather
    hourWeather.image = char(64);
    hourWeather.temp = 0;
    hourWeather.windbft = 0;
    hourWeather.windrgr = 0;
    hourWeather.neersl = 0;
  }
  return hourWeather;
}

size_t Weerlive::getHourCount() {
  return response["uur_verw"].as<JsonArray>().size();
}
