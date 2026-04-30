#include "Arduino.h"
#include "WiFi.h"
#include <WiFiAP.h>
#include "WebServer.h"
#include "Audio.h"

// Digital I/O used
#define I2S_DOUT      4
#define I2S_BCLK      5
#define I2S_LRC       6


#include "secrets.h"
String ssid = SECRET_SSID;
String pass = SECRET_PASS;
const char *ssidAP = SECRET_SSID_AP;

//Current status of the network (IP network)
boolean networkAvailable = false;

Audio audio;

WebServer server(80);

void handleWifiConfig() {
  server.send(200,"text/html","<h1>Wifi config</h1>");
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

void setupWiFiAccessPoint() {
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

  // wait 2 seconds for connection:
  delay(2000);

}

void setupWiFi() {
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
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(15); // default 0...21
  audio.connecttohost("http://stream.antennethueringen.de/live/aac-64/stream.antennethueringen.de/");
}

void setupWebServer() {
  server.on("/", handleRoot);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  initAudio();
  Serial.println("");
  //Try a connection to the WiFI
  setupWiFi();
  if (networkAvailable) {
    setupAudio();
    setupWebServer();
  } else {
    //If Wifi setup fails, create an access point (to initialize everything)
    setupWiFiAccessPoint();
    server.on("/", handleWifiConfig);
  }
  server.begin();
}

void loop(){
  if (networkAvailable) {
    audio.loop();
  }
  server.handleClient();
  vTaskDelay(1);
}
