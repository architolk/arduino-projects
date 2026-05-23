#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <Audio.h>
#include <ArduinoJson.h>

#include <esp_ota_ops.h>
#include <ElegantOTA.h>

#include "debug.h"
#include "config-page.h"
#include "events-page.h"
#include "radio-page.h"
#include "radiostations.h"
#include "css-file.h"

// Digital I/O used
#define I2S_DOUT      21
#define I2S_BCLK      47
#define I2S_LRC       48

#include "secrets.h"
const char *ssidAP = SECRET_SSID_AP;
#define MAX_CONNECT_TRIES 10

//Needed for checking sensors every 0.5 second
unsigned long previousMillis = 0;
#define SENSOR_INTERVAL 500
#define SENSOR_STARTUP 5000
#define TOUCH_SENSITIVITY 80000

//Current status of the network (IP network)
boolean networkAvailable = false;
boolean shouldConnectToWiFi = false;
boolean audioAvailable = false;

Audio audio;

AsyncWebServer server(80);
AsyncEventSource events("/api/events");
DNSServer dnsServer;
char sseBuffer[200];
Preferences prefs;

int currentVolume = 0;
int currentBass = 0;
int currentTreble = 0;
int currentFreq = 0;
int currentPreset = 0; //FM Manual;

#define MAXNETWORKS 10
String networks[MAXNETWORKS];
int networkCount = 0;

String buildConfigHtml() {
  // Injecteer <option> regels in de datalist placeholder
  String html = FPSTR(CONFIG_HTML);
  String opts;
  for (int i = 0; i < networkCount; i++) {
    // datalist gebruikt <option value="...">
    opts += "<option value=\"";
    opts += networks[i];
    opts += "\"></option>\n";
  }
  html.replace("<!--OPTIONS-->", opts);
  return html;
}

String buildRadioStationsHtml() {
  // Dit kan eigenlijk beter met een afzonderlijke JSON API, waardoor de originel pagina statisch blijft...
  String html = FPSTR(RADIO_HTML);
  String opts;
  for (int i = 0; i < stationCount; i++) {
    // radiobuttons
    opts += "<label for='rs"+ String(i) +"' class='scroll-item'>";
    opts += stations[i].url;
    opts += "<span>" + String(0.1*stations[i].freq,1) + "</span>";
    opts += "<input id='rs"+String(i)+"' type='radio' name='stations'></label>\n";
  }
  html.replace("<!--OPTIONS-->", opts);
  return html;
}

void handleEventsPage(AsyncWebServerRequest *request) {
  request->send(200,"text/html",EVENTS_HTML);
}

void handleWifiConfigAPI(AsyncWebServerRequest *request) {
  String body = request->arg("plain");
  if (body.length() == 0) {
    request->send(400, "text/plain", "Missing body");
    return;
  }

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    request->send(400, "text/plain", "Invalid JSON");
    return;
  }

  const char* ssid = doc["ssid"];
  const char* password = doc["password"];
  if (!ssid || !password || String(ssid).length() == 0) {
    request->send(400, "text/plain", "Missing ssid/password");
    return;
  }

  // Opslaan in NVS (Preferences)
  prefs.begin("wifi", false);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", password);
  prefs.end();

  // Let's retry connecting to the WiFi (handles in the loop)
  shouldConnectToWiFi = true;

  // Succes
  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleRoot(AsyncWebServerRequest *request) {
  if (networkAvailable) {
    //server.send(200,"text/html","<h1>Hello from the radio</h1>");
    request->send(200,"text/html",buildRadioStationsHtml());
  } else {
    request->send(200,"text/html",buildConfigHtml());
  }
}

void handleCSS(AsyncWebServerRequest *request) {
  request->send(200,"text/css",MAIN_CSS);
}

// callbacks
void my_audio_info(Audio::msg_t m) {
  Debug(m.s);
  Debug(": ");
  Debugln(m.msg);

  //New option for web event streaming
  switch (m.e) {
    case Audio::evt_name:
      events.send(m.msg,"station");
      //Station found
      digitalWrite(39,HIGH);
      break;
    case Audio::evt_streamtitle:
      events.send(m.msg,"song");
      break;
    default:
      sprintf(sseBuffer,"%s: %s\n\n", m.s, m.msg);
      events.send(sseBuffer,"info");
  }
}

void initAudio() {
  Audio::audio_info_callback = my_audio_info; // optional
}

void initSensors() {
  previousMillis = millis() + SENSOR_STARTUP; //Wait some time before sensor reading starts
  pinMode(1,INPUT);
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  pinMode(4,INPUT);
  pinMode(5,INPUT);
  pinMode(6,INPUT);
  pinMode(7,INPUT_PULLUP);
  pinMode(8,INPUT);
  pinMode(9,INPUT);
  pinMode(10,INPUT);
  pinMode(11,INPUT);
  pinMode(12,INPUT);
  pinMode(13,INPUT);
  pinMode(14,INPUT);
  pinMode(39,OUTPUT);
  digitalWrite(39,LOW);
}

void setupWiFiAccessPoint() {
  Debugln("Scanning for networks");
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  networkCount = min(n, MAXNETWORKS);
  for (int i=0; i<networkCount; i++) {
    networks[i] = WiFi.SSID(i);
    Debugln("- " + networks[i]);
  }
  WiFi.mode(WIFI_AP);
  // print the network name (SSID);
  Debug("Creating access point named: ");
  Debugln(ssidAP);

  // Create open network.
  if (!WiFi.softAP(ssidAP)) {
    Debugln("Soft AP creation failed.");
    while (1);
  }
  Debug("AP IP address: ");
  Debugln(WiFi.softAPIP());

  // Wildcard DNS every hostname -> ESP32
  dnsServer.start(53, "*", WiFi.softAPIP());

}

void setupWiFi() {
  prefs.begin("wifi", true);
  String ssid = prefs.getString("ssid","");
  String pass = prefs.getString("pass","");
  prefs.end();
  if (ssid=="") return; // nothing saved yet

  // attempt to connect to WiFi network:
  Debug("Attempting to connect to SSID: ");
  Debugln(ssid);
  WiFi.begin(ssid.c_str(), pass.c_str());
  int maxTry = 0;
  networkAvailable = false;
  while ((WiFi.status() != WL_CONNECTED) && (maxTry < MAX_CONNECT_TRIES)) {
    Debug(".");
    maxTry++;
    delay(1500);
  }
  Debugln("");
  if (WiFi.status() == WL_CONNECTED) {
    Debugln("Connected");
    networkAvailable = true;
  }
}

void setupAudio() {
  Debug("DOUT: ");
  Debug(I2S_DOUT);
  Debug(" BCLK: ");
  Debug(I2S_BCLK);
  Debug(" LRC: ");
  Debugln(I2S_LRC);
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  /*
  audio.setVolumeCurve([](float t) {
    //return -60.0f + 60.0f * t;
    return -60.8f + 60.8f * sqrt(t);
  });
  */
  audio.setVolume(15); // default 0...21

  Station* station;
  if (getStation(900,&station)) {
    audio.connecttohost(station->url);
  } else {
    //Connect to this radiostation by default
    audio.connecttohost("http://stream.antennethueringen.de/live/aac-64/stream.antennethueringen.de/");
  }
  audioAvailable = true;
}

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/main.css", handleCSS);
  server.on("/events", handleEventsPage);
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected!");
    }
  });
  server.addHandler(&events);
}

void setupIPName() {
  if (MDNS.begin(ssidAP)) {
    Debug("mDNS started. Access at http://");
    Debug(ssidAP);
    Debugln(".local");
  } else {
    Debugln("Error starting mDNS");
  }
}

void debugPartitionInfo() {
  const esp_partition_t *app = esp_ota_get_running_partition();

  Debug("Running partition: ");
  Debugln(app->label);
  Debug("Address: ");
  Debugln(app->address);
  Debug("Size: ");
  Debugln(app->size / 1024);
}

void setup() {
  initSensors();
  DebugBegin(115200);
  delay(1000);
  Debugln("");
  Debugln("Starting");
  debugPartitionInfo();
  initAudio();
  loadStations();
  //Try a connection to the WiFI
  setupWiFi();
  setupWebServer();
  if (networkAvailable) {
    setupAudio();
  } else {
    //If Wifi setup fails, create an access point (to initialize everything)
    setupWiFiAccessPoint();
    server.on("/api/wificonfig", handleWifiConfigAPI);

    //Captive-portal probes (Android, iOS, Windows)
    /*
    server.on("/generate_204", [](){ redirectRoot(); });
    server.on("/hotspot-detect.html",[](){ redirectRoot(); });
    server.on("/ncsi.txt", [](){ server.send(200,"text/plain","Microsoft NCSI"); });
    server.on("/connecttest.txt", [](){ server.send(200,"text/plain",""); });
    server.on("/fwlink", [](){ redirectRoot(); });
    server.onNotFound([](){ handleRoot(); });
    */
  }
  setupIPName();
  server.begin();
  ElegantOTA.begin(&server);
}

void redirectRoot(AsyncWebServerRequest *request) {
  request->send(200,"text/html","<html><body><script>location.href='/'</script></body></html>");
}

void checkSensors() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= SENSOR_INTERVAL) {
    previousMillis = currentMillis;
    checkTouch();
    checkAudioSettings();
  }
}

void checkAudioSettings() {
  float value = analogRead(4);
  value = powf(value/4095.0f,0.31f)*4095.0f; //Compensate for log potentiometer
  int volume = map(value,0,4095,0,21);
  if (volume!=currentVolume) {
    currentVolume = volume;
    Debug("New volume: ");
    Debugln(currentVolume);
    sprintf(sseBuffer,"%d/%d",currentVolume,value);
    events.send(sseBuffer,"volume");
    if (audioAvailable) {
      audio.setVolume(currentVolume);
    }
  }
  int treble = map(analogRead(5),0,4095,12,-12); //Treble potentiometer is wired in reverse
  if (treble!=currentTreble) {
    currentTreble = treble;
    Debug("New treble: ");
    Debugln(currentTreble);
    if (audioAvailable) {
      audio.setTone(currentBass,0,currentTreble);
    }
  };
  int bass = map(analogRead(6),0,4095,-12,12);
  if (bass!=currentBass) {
    currentBass = bass;
    Debug("New bass: ");
    Debugln(currentBass);
    if (audioAvailable) {
      audio.setTone(currentBass,0,currentTreble);
    }
  };
}

void checkTouch() {
  int freq = 0;
  bool t11 = (touchRead(11)>TOUCH_SENSITIVITY);
  bool t12 = (touchRead(12)>TOUCH_SENSITIVITY);
  bool t13 = (touchRead(13)>TOUCH_SENSITIVITY);
  bool t14 = (touchRead(14)>TOUCH_SENSITIVITY);
  bool t7 = (digitalRead(7)==HIGH);
  if (t11 && (!t12)) {
    currentPreset = 3;
    freq = map(analogRead(9),0,4095,870,1080);
    Debug("Preset 3: ");
    Debugln(freq);
    //Show LED
    pinMode(42,INPUT);
    pinMode(40,OUTPUT);
    pinMode(41,OUTPUT);
    digitalWrite(40,LOW);
    digitalWrite(41,HIGH);

  };
  if (t12 && (!t11)) {
    currentPreset = 2;
    freq = map(analogRead(3),0,4095,870,1080);
    Debug("Preset 2: ");
    Debugln(freq);
    //Show LED
    pinMode(41,INPUT);
    pinMode(42,OUTPUT);
    pinMode(40,OUTPUT);
    digitalWrite(42,LOW);
    digitalWrite(40,HIGH);
  };
  if (t13) {
    currentPreset = 0;
    freq = map(analogRead(1),0,4095,870,1080);
    Debug("Preset MAN: ");
    Debug(freq); //FM tuning
    Debug(" / ");
    Debug(map(analogRead(2),0,4095,870,1080)); //FM fine tuning
    if (t7) { //FM Lock
      Debugln(" (on)");
    } else {
      Debugln(" (off)");
    }
    //Show LED
    pinMode(42,INPUT);
    pinMode(41,OUTPUT);
    pinMode(40,OUTPUT);
    digitalWrite(41,LOW);
    digitalWrite(40,HIGH);
  };
  if (t14) {
    currentPreset = 1;
    freq = map(analogRead(8),0,4095,870,1080);
    Debug("Preset 1: ");
    Debugln(freq);
    //Show LED
    pinMode(40,INPUT);
    pinMode(42,OUTPUT);
    pinMode(41,OUTPUT);
    digitalWrite(42,LOW);
    digitalWrite(41,HIGH);
  };
  if (t11 && t12) {
    currentPreset = 4;
    freq = map(analogRead(10),0,4095,870,1080);
    Debug("Preset 4: ");
    Debugln(freq);
    //Show LED
    pinMode(40,INPUT);
    pinMode(41,OUTPUT);
    pinMode(42,OUTPUT);
    digitalWrite(41,LOW);
    digitalWrite(42,HIGH);
  }
  if (t7 && (currentPreset==0)) {
    freq = map(analogRead(1),0,4095,870,1080);
  }
  if (freq!=0) {
    Station* station;
    if (getStation(freq,&station)) {
      if (currentFreq!=station->freq) { //Only switch if a different station is selected!
        currentFreq = station->freq;
        Debug(freq);
        Debug(": ");
        Debugln(station->url);
        if (audioAvailable) {
          digitalWrite(39,LOW); //Reset tuning light
          audio.connecttohost(station->url);
        }
      }
    }
  }
}

void loop(){
  if (networkAvailable) {
    audio.loop();
  } else {
    dnsServer.processNextRequest();
    if (shouldConnectToWiFi) {
      shouldConnectToWiFi = false;
      setupWiFi();
      if (networkAvailable) {
        setupAudio();
        setupWebServer();
      }
    }
  }
  checkSensors();
  ElegantOTA.loop();
  vTaskDelay(1);
}
