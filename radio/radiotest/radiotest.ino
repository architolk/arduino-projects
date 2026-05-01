#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <Audio.h>
#include <ArduinoJson.h>
#include "config-page.h"

// Digital I/O used
#define I2S_DOUT      4
#define I2S_BCLK      5
#define I2S_LRC       6


#include "secrets.h"
const char *ssidAP = SECRET_SSID_AP;

//Current status of the network (IP network)
boolean networkAvailable = false;
boolean shouldConnectToWiFi = false;

Audio audio;

WebServer server(80);
DNSServer dnsServer;
Preferences prefs;

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
  server.handleClient();
  vTaskDelay(1);
}
