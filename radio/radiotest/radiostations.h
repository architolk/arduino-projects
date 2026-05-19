#ifndef _RADIOSTATIONS_H
#define _RADIOSTATIONS_H

#include <LittleFS.h>

#include "debug.h"

//Default radiostations. Will be used when no radiostations file can be found in LittleFS memory of the ESP32

//Any number of radiostations is possible (might max out in the global variables)
//Frequency should be between 87.5 and 108.0
//Any distance between stations is possible, but might create problems with selecting a particular station - 0.5 is probably safe
//Format should be: Frequency <space> URL <space> Description

#define MAX_STATIONS 40
#define MAX_URL_LENGTH 100
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

const char RADIOSTATIONS[] PROGMEM = R"=====(
87.5 https://stream.qmusic.nl/fouteuur/aachigh Q Foute uur
88.0 https://icecast.omroep.nl/radio1-bb-aac NPO Radio 1
89.0 https://icecast.omroep.nl/radio2-bb-aac NPO Radio 2
90.0 https://icecast.omroep.nl/3fm-bb-aac NPO 3FM
91.0 https://icecast.omroep.nl/radio4-bb-aac NPO Klassiek
92.0 https://icecast.omroep.nl/radio5-bb-aac NPO Radio 5
93.0 https://icecast.omroep.nl/radio6-bb-aac NPO Soul and Jazz
94.0 https://stream.radionl.fm/rnltwente Radio NL Twente
95.0 http://stream.player.arrow.nl/arrowcr Arrow classic rock
96.0 https://stream.accentfm.nl/ Radio Noord-oost Twente
108.0 http://stream.frysk.fm/fryskfm Frysk FM
)=====";

struct Station {
  int freq;
  char url[MAX_URL_LENGTH];
};
Station stations[MAX_STATIONS];
int stationCount = 0;

void initStations() {
  for (int i=0; i<MAX_STATIONS; i++) {
    stations[i].freq = 0; //0 means station not in use
  }
}

void safeCopy(char* dst, size_t dstSize, const char* src) {
  if (dstSize == 0) return;
  std::strncpy(dst, src, dstSize-1);
  dst[dstSize-1] = '\0';
}

bool addStation(int freq, const char* url) {
  if (!url) return false;
  if (stationCount >= MAX_STATIONS) return false;
  int pos = 0;
  while (pos < stationCount && stations[pos].freq < freq) { pos++; }
  if (pos < stationCount) {
    memmove(&stations[pos+1], &stations[pos], (stationCount-pos)*sizeof(Station));
  }
  stations[pos].freq = freq;
  safeCopy(stations[pos].url,sizeof(stations[pos].url),url);
  stationCount++;

  Debug("Station #");
  Debug(pos);
  Debug(": ");
  Debug(stations[pos].freq);
  Debug(" ");
  Debugln(stations[pos].url);
  return true;
}

//Load stations from PROGMEM
void loadStations() {
  float freq;
  char url[MAX_URL_LENGTH];
  size_t i = 0;
  size_t pos = 0;
  char buffer[MAX_URL_LENGTH];
  bool dataAvailable = true;

  while (dataAvailable) {
    char c = pgm_read_byte(&RADIOSTATIONS[i++]);

    if ((c == '\0') || (c == '\n')) {
        buffer[pos] = '\0';
        if (sscanf(buffer,"%f %99s", &freq, url) == 2) {
          addStation(freq*10,url);
          pos = 0;
        }
    } else {
      if (pos + 1 < MAX_URL_LENGTH) {
          buffer[pos++] = c;
      }
    }
    dataAvailable = (c!='\0');
  }
}

//Load stations from LittleFS
void readStations() {
  //TODO: File systeme initializing - haven't done this yet - LittleFS.begin()
  float freq;
  char url[MAX_URL_LENGTH];
  char buffer[MAX_URL_LENGTH];
  File file = LittleFS.open("/stations.txt",FILE_READ);
  if (!file) {
    Debugln("Opening file failed");
    return;
  }

  while (file.available()) {
    file.readBytesUntil('\n',buffer,MAX_URL_LENGTH); //Doesn't exists! waarschijnlijk gelijk doen aan PROGMEM...
    if (sscanf(buffer,"%f %99s", &freq, url) == 2) {
      addStation(freq*10,url);
    }
  }
  file.close();
}

//Write stations to LittleFS
void writeStations() {
  char buffer[MAX_URL_LENGTH];
  File file = LittleFS.open("/stations.txt",FILE_WRITE);
  if (!file) {
    Debugln("Writing file failed");
    return;
  }
  for (int i=0; i<stationCount; i++) {
    sprintf(buffer,"%f %99s",1.0f*stations[i].freq/10.0f,stations[i].url);
    file.println(buffer);
  }
  file.close();
}

bool getStation(int freq, Station** station) {
  if (stationCount==0) return false;
  int bestIndex = 0;
  int smallest = abs(stations[0].freq - freq);
  for (int i=1; i < stationCount; i++) {
    int diff = abs(stations[i].freq - freq);
    if (diff < smallest) {
      smallest = diff;
      bestIndex = i;
    } else {
      break; //Sorted list, so nothing better in the rest of the list
    }
  }
  *station = &stations[bestIndex];
  return true;
}

#endif
