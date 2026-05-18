#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <Audio.h>
#include <ArduinoJson.h>
#include "config-page.h"
#include "radiostations.h"

// Digital I/O used
/*
//This works (but uses the ADC_1)
#define I2S_DOUT      4
#define I2S_BCLK      5
#define I2S_LRC       6
//This works, but dropouts...
#define I2S_DOUT      15
#define I2S_BCLK      16
#define I2S_LRC       17
*/
//This also works (but strange behaviour - use this probably...)
#define I2S_DOUT      21
#define I2S_BCLK      47
#define I2S_LRC       48

#include "secrets.h"
const char *ssidAP = SECRET_SSID_AP;

//Needed for checking sensors every 0.5 second
unsigned long previousMillis = 0;
#define SENSOR_INTERVAL 500
#define SENSOR_STARTUP 5000

//Current status of the network (IP network)
boolean networkAvailable = false;
boolean shouldConnectToWiFi = false;
boolean audioAvailable = false;

Audio audio;

WebServer server(80);
DNSServer dnsServer;
Preferences prefs;

int currentVolume, newVolume = 0;
int currentBass, newBass = 0;
int currentTreble, newTreble = 0;
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


void handleWifiConfig() {
  //server.send(200,CONFIG_HTML);
  server.send(200,"text/html",buildConfigHtml());
}

void handleWifiConfigAPI() {
  String body = server.arg("plain");
  if (body.length() == 0) {
    server.send(400, "text/plain", "Missing body");
    return;
  }

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  const char* ssid = doc["ssid"];
  const char* password = doc["password"];
  if (!ssid || !password || String(ssid).length() == 0) {
    server.send(400, "text/plain", "Missing ssid/password");
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
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleRoot() {
  server.send(200,"text/html","<h1>Hello from the radio</h1>");
}

// callbacks
void my_audio_info(Audio::msg_t m) {
    Serial.printf("%s: %s\n", m.s, m.msg);
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
  //pinMode(39,OUTPUT);
}

void setupWiFiAccessPoint() {
  Serial.println("Scanning for networks");
  WiFi.mode(WIFI_AP_STA);
  int n = WiFi.scanNetworks();
  networkCount = min(n, MAXNETWORKS);
  for (int i=0; i<networkCount; i++) {
    networks[i] = WiFi.SSID(i);
    Serial.println("- " + networks[i]);
  }
  WiFi.mode(WIFI_AP);
  // print the network name (SSID);
  Serial.print("Creating access point named: ");
  Serial.println(ssidAP);

  // Create open network.
  if (!WiFi.softAP(ssidAP)) {
    Serial.println("Soft AP creation failed.");
    while (1);
  }
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

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
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid.c_str(), pass.c_str());
  int maxTry = 0;
  networkAvailable = false;
  while ((WiFi.status() != WL_CONNECTED) && (maxTry < 10)) {
    Serial.print(".");
    maxTry++;
    delay(1500);
  }
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected");
    networkAvailable = true;
  }
}

void setupAudio() {
  Serial.print("DOUT: ");
  Serial.print(I2S_DOUT);
  Serial.print(" BCLK: ");
  Serial.print(I2S_BCLK);
  Serial.print(" LRC: ");
  Serial.println(I2S_LRC);
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  //Linear volume curve (as the potentiometer is already exponential)
  /*
  audio.setVolumeCurve([](float t) {
    return -60.0f + 60.0f * t;
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
}

void setup() {
  initSensors();
  Serial.begin(115200);
  delay(1000);
  initAudio();
  Serial.println("");
  Serial.println("Starting");
  //Try a connection to the WiFI
  //setupWiFi(); TEMPORARY FOR DEBUGGING
  if (networkAvailable) {
    loadStations();
    setupAudio();
    setupWebServer();
  } else {
    //If Wifi setup fails, create an access point (to initialize everything)
    setupWiFiAccessPoint();
    server.on("/", handleWifiConfig);
    server.on("/api/wificonfig", handleWifiConfigAPI);

    //Captive-portal probes (Android, iOS, Windows)
    server.on("/generate_204", [](){ redirectRoot(); });
    server.on("/hotspot-detect.html",[](){ redirectRoot(); });
    server.on("/ncsi.txt", [](){ server.send(200,"text/plain","Microsoft NCSI"); });
    server.on("/connecttest.txt", [](){ server.send(200,"text/plain",""); });
    server.on("/fwlink", [](){ redirectRoot(); });
    server.onNotFound([](){ handleWifiConfig(); });
  }
  server.begin();
}

void redirectRoot() {
  server.send(200,"text/html","<html><body><script>location.href='/'</script></body></html>");
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
  int volume = map(analogRead(4),0,4095,0,21);
  if ((volume==newVolume) && (volume!=currentVolume)) {
    currentVolume = volume;
    Serial.print("New volume: ");
    Serial.println(currentVolume);
    if (audioAvailable) {
      audio.setVolume(currentVolume);
    }
  } else {
    newVolume = volume; //When volume stays the same for .5 seconds, the volume will change
  };
  int treble = map(analogRead(5),0,4095,-12,12);
  if ((treble==newTreble) && (treble!=currentTreble)) {
    currentTreble = treble;
    Serial.print("New treble: ");
    Serial.println(currentTreble);
    if (audioAvailable) {
      audio.setTone(currentBass,0,currentTreble);
    }
  } else {
    newTreble = treble;
  };
  int bass = map(analogRead(6),0,4095,-12,12);
  if ((bass==newBass) && (bass!=currentBass)) {
    currentBass = bass;
    Serial.print("New bass: ");
    Serial.println(currentBass);
    if (audioAvailable) {
      audio.setTone(currentBass,0,currentTreble);
    }
  } else {
    newBass = bass;
  };
}

void checkTouch() {
  int freq = 0;
  bool t11 = (touchRead(11)>100000);
  bool t12 = (touchRead(12)>100000);
  bool t13 = (touchRead(13)>100000);
  bool t14 = (touchRead(14)>100000);
  bool t7 = (digitalRead(7)==HIGH);
  if (t11 && (!t12)) {
    currentPreset = 3;
    freq = map(analogRead(9),0,4095,870,1080);
    Serial.print("Preset 3: ");
    Serial.println(freq);
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
    Serial.print("Preset 2: ");
    Serial.println(freq);
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
    Serial.print("Preset MAN: ");
    Serial.print(freq); //FM tuning
    Serial.print(" / ");
    Serial.print(map(analogRead(2),0,4095,87,108)); //FM fine tuning
    if (t7) { //FM Lock
      Serial.println(" (on)");
    } else {
      Serial.println(" (off)");
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
    Serial.print("Preset 1: ");
    Serial.println(freq);
    //Show LED
    pinMode(40,INPUT);
    pinMode(42,OUTPUT);
    pinMode(41,OUTPUT);
    digitalWrite(42,LOW);
    digitalWrite(41,HIGH);
  };
    currentPreset = 4;
  if (t11 && t12) {
    freq = map(analogRead(10),0,4095,870,1080);
    Serial.print("Preset 4: ");
    Serial.println(freq);
    //Show LED
    pinMode(40,INPUT);
    pinMode(41,OUTPUT);
    pinMode(42,OUTPUT);
    digitalWrite(41,LOW);
    digitalWrite(42,HIGH);
  }
  /*
  if (t7 && (currentPreset==0)) {
    freq = map(analogRead(1),0,4095,870,1080);
    if (freq==currentFreq) {
      freq = 0; //Don't do anything if the frequency is still the same
    }
  }
  */
  if (freq!=0) {
    Station* station;
    if (getStation(freq,&station)) {
      Serial.println(station->url);
      if (audioAvailable) {
        audio.connecttohost(station->url);
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
        loadStations();
        setupAudio();
        setupWebServer();
      }
    }
  }
  checkSensors();
  server.handleClient();
  vTaskDelay(1);
}
