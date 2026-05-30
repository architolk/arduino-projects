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
#include "upload-page.h"
#include "radiostations.h"
#include "css-file.h"

//Should only be true ones (at initial installation)
#define FORMAT_LITTLEFS_IF_FAILED false

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
char stationNameBuffer[200];
char songTitleBuffer[200];
Preferences prefs;

int currentVolume = 0;
int currentBass = 0;
int currentTreble = 0;
int currentFreq = 0;
int currentPreset = 0; //FM Manual;

bool easterEggMode = false;
int flashLEDPosition = 0;
int flashLEDDirection = 1;
unsigned long previousEggMillis = 0;
#define FLASH_INTERVAL 200

#define MAXNETWORKS 10
String networks[MAXNETWORKS];
int networkCount = 0;

String processNetworks(const String& var) {
  if (var=="OPTIONS") {
    String opts;
    for (int i = 0; i < networkCount; i++) {
      // datalist gebruikt <option value="...">
      opts += "<option value=\"";
      opts += networks[i];
      opts += "\"></option>\n";
    }
    return opts;
  }
  return String();
}

String processRadioStations(const String& var) {
  if (var=="OPTIONS") {
    String opts;
    for (int i = 0; i < stationCount; i++) {
      // radiobuttons
      opts += "<label class='scroll-item'>";
      opts += stations[i].url;
      opts += "<span>" + String(0.1*stations[i].freq,1) + "</span>";
      opts += "<input type='radio' name='stations'></label>\n";
    }
    return opts;
  }
  return String();
}

String processEvents(const String& var) {
  if (var=="STATIONNAME") {
    return String(stationNameBuffer);
  }
  if (var=="SONGTITLE") {
    return String(songTitleBuffer);
  }
  if (var=="VOLUME") {
    return String(currentVolume);
  }
  if (var=="TREBLE") {
    return String(currentTreble);
  }
  if (var=="BASS") {
    return String(currentBass);
  }
  return String();
}

void handleEventsPage(AsyncWebServerRequest *request) {
  request->send_P(200,"text/html",EVENTS_HTML,processEvents);
}

void handleStationsFile(AsyncWebServerRequest *request) {
  if (LittleFS.exists("/stations.txt")) {
    request->send(LittleFS, "/stations.txt", String(), true);
  } else {
    request->send(400, "text/plain", "File not found");
  }
}

void handleUploadPage(AsyncWebServerRequest *request) {
  request->send_P(200,"text/html",UPLOAD_HTML);
}

void handleRadioEditAPI(AsyncWebServerRequest *request, JsonVariant &json) {
  const JsonObject obj = json.as<JsonObject>();
  if (!obj) {
    request->send(400, "text/plain", "Gegevens ontbreken");
    return;
  }
  const int action = obj["action"];
  const int freq = obj["freq"];
  const int newFreq = obj["newfreq"];
  const char* url = obj["url"];
  if (!url || String(url).length() == 0 ) {
    request->send(400, "text/plain", "Waarden ontbreken");
    return;
  }
  if ((freq<875) || (freq>1080)) {
    request->send(400, "text/plain", "De frequentie is niet tussen 87.5 en 108.0");
  }
  if (action==3) {
    if ((newFreq<875) || (newFreq>1080)) {
      request->send(400, "text/plain", "De frequentie is niet tussen 87.5 en 108.0");
    }
    if (updateStation(freq,newFreq,url)) {
      request->send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      request->send(400, "text/plain", "Fout bij aanpassen radiostation");
    }
    return;
  }
  if (action==2) {
    if (addStation(freq,url)) {
      request->send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      request->send(400, "text/plain", "Fout bij toevoegen radiostation");
    }
    return;
  }
  if (action==1) {
    if (deleteStation(freq)) {
      request->send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      request->send(400, "text/plain", "Fout bij verwijderen radiostation");
    }
    return;
  }
  request->send(400, "text/plain", "Onbekende actie");
  return;
}

void handleUploadAPI(AsyncWebServerRequest *request) {
  if (request->getResponse()) {
    // 400 File not available for writing (done in handleUploadFileAPI)
    return;
  }
  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleUploadFileAPI(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  Debug("Upload [");
  Debug(filename);
  Debug("]: start=");
  Debug(index);
  Debug(", len=");
  Debug(len);
  if (final) {Debugln(" (final)");} else {Debugln(" (continue)");}
  //index==0 => Start of file upload
  if (index == 0) {
    request->_tempFile = LittleFS.open("/stations.txt", "w");
  }
  if (!request->_tempFile) {
    request->send(400, "text/plain", "Kon stations.txt niet wegschrijven");
    return;
  }
  if (len>0) {
    request->_tempFile.write(data, len);
  }
  //final==true => End of file upload
  if (final) {
    request->_tempFile.close();
  }
}

void handleDeleteFileAPI(AsyncWebServerRequest *request, JsonVariant &json) {
  if (!LittleFS.remove("/stations.txt")) {
    request->send(400, "text/plain", "Kon stations.txt niet verwijderen");
  } else {
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  }
}

void handleStationsSaveAPI(AsyncWebServerRequest *request, JsonVariant &json) {

  if (writeStations()) {
    // Succes
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    request->send(400, "text/plain", "Fout bij opslaan radiostations");
  }
}

void handleWifiConfigAPI(AsyncWebServerRequest *request, JsonVariant &json) {

  const JsonObject obj = json.as<JsonObject>();
  if (!obj) {
    request->send(400, "text/plain", "Gegevens ontbreken");
    return;
  }
  const char* ssid = obj["ssid"];
  const char* password = obj["password"];
  if (!ssid || !password || String(ssid).length() == 0) {
    request->send(400, "text/plain", "ssid/password ontbreekt");
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
    request->send_P(200,"text/html",EVENTS_HTML,processEvents);
  } else {
    request->send_P(200,"text/html",CONFIG_HTML,processNetworks);
  }
}

void handleWifiConfig(AsyncWebServerRequest *request) {
  request->send_P(200,"text/html",CONFIG_HTML,processNetworks);
}

void handleStationsPage(AsyncWebServerRequest *request) {
  request->send_P(200,"text/html",RADIO_HTML,processRadioStations);
}

void handleCSS(AsyncWebServerRequest *request) {
  request->send_P(200,"text/css",MAIN_CSS);
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
      safeCopy(stationNameBuffer,sizeof(stationNameBuffer),m.msg);
      //Station found
      digitalWrite(39,HIGH);
      break;
    case Audio::evt_streamtitle:
      events.send(m.msg,"song");
      safeCopy(songTitleBuffer,sizeof(songTitleBuffer),m.msg);
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
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
    delay(750);
    rgbLedWrite(RGB_BUILTIN, 255,0,0);
    delay(750);
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
  audio.setVolume(15); // default 0...21

  audioAvailable = true;
  setRadioStation(getFrequency(1,true));
}

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/main.css", handleCSS);
  server.on("/events", handleEventsPage);
  server.on("/stations",handleStationsPage);
  server.on("/wificonfig",handleWifiConfig);
  server.on("/upload",handleUploadPage);
  server.on("/api/upload",HTTP_POST, handleUploadAPI,handleUploadFileAPI);
  server.on("/stations.txt",handleStationsFile);
  server.addHandler(new AsyncCallbackJsonWebHandler("/api/wificonfig",handleWifiConfigAPI));
  server.addHandler(new AsyncCallbackJsonWebHandler("/api/stations",handleRadioEditAPI));
  server.addHandler(new AsyncCallbackJsonWebHandler("/api/savestations",handleStationsSaveAPI));
  server.addHandler(new AsyncCallbackJsonWebHandler("/api/deletefile",handleDeleteFileAPI));
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Debugln("Client reconnected!");
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

void setupStations() {
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Debugln("LittleFS Mount Failed");
    loadStations(); //Fall back to predefined stations
    return;
  }
  if (!readStations()) {
    loadStations(); //Fall back to predefined stations
  }
}

void calibrateSensors() {
  //First reading of touch sensors is incorrect, so drop these
  bool t11 = (touchRead(11)>TOUCH_SENSITIVITY);
  bool t12 = (touchRead(12)>TOUCH_SENSITIVITY);
  bool t13 = (touchRead(13)>TOUCH_SENSITIVITY);
  bool t14 = (touchRead(14)>TOUCH_SENSITIVITY);
  //Actual calibrating routine is not performed... TODO...
  showLED(currentPreset); //currentPreset = 0, FM manual at startup
}

void setup() {
  stationNameBuffer[0] = '\0';
  songTitleBuffer[0] = '\0';
  rgbLedWrite(RGB_BUILTIN, 255, 0, 0);  // Red - indicating we've got lift-off
  initSensors();
  DebugBegin(115200);
  delay(1000);
  Debugln("");
  Debugln("Starting");
  Debug("RGB Buildtin pin: ");
  Debugln(RGB_BUILTIN);
  calibrateSensors();
  debugPartitionInfo();
  initAudio();
  setupStations();
  //Try a connection to the WiFI
  setupWiFi();
  rgbLedWrite(RGB_BUILTIN,0,0,0); //LED off
  setupWebServer();
  if (networkAvailable) {
    setupAudio();
  } else {
    //If Wifi setup fails, create an access point (to initialize everything)
    rgbLedWrite(RGB_BUILTIN, 0, 165, 0); //Setting up Wifi access-point
    setupWiFiAccessPoint();

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

void checkEasterEggMode() {
  //Toggle LEDs when in easter egg mode.
  if (easterEggMode) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousEggMillis >= FLASH_INTERVAL) {
      previousEggMillis = currentMillis;
      flashLEDPosition = flashLEDPosition + flashLEDDirection;
      if (flashLEDPosition>=4) flashLEDDirection = -1;
      if (flashLEDPosition<=0) flashLEDDirection = 1;
      showLED(flashLEDPosition);
    }
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
    sprintf(sseBuffer,"%d",currentVolume);
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
    sprintf(sseBuffer,"%d",currentTreble);
    events.send(sseBuffer,"treble");
    if (audioAvailable) {
      audio.setTone(currentBass,0,currentTreble);
    }
  };
  int bass = map(analogRead(6),0,4095,-12,12);
  if (bass!=currentBass) {
    currentBass = bass;
    Debug("New bass: ");
    Debugln(currentBass);
    sprintf(sseBuffer,"%d",currentBass);
    events.send(sseBuffer,"bass");
    if (audioAvailable) {
      audio.setTone(currentBass,0,currentTreble);
    }
  };
}

int getFrequency(int pin, bool fineTuning) {
  float value1 = analogRead(pin); vTaskDelay(1);
  float value2 = analogRead(pin); vTaskDelay(1);
  float value3 = analogRead(pin);
  float value = (value1+value2+value3)/3.0f; //Take average, compensate for measure inconsistancies
  value = powf(value/4095.0f,0.517f)*4095.0f; //Compensate potentiometer
  if (fineTuning) {
    float fine1 = analogRead(2); vTaskDelay(1);
    float fine2 = analogRead(2); vTaskDelay(1);
    float fine3 = analogRead(2);
    float fine = (fine1+fine2+fine3)/3.0f;
    return 10*map(value,0,4095,85,110) + map(fine,0,4095,0,9); //Fine tuning does .x value
  } else {
  return map(value,0,4095,850,1100);  }
}

void showLED(int led) {
  switch (led) {
    case 0: {
      pinMode(42,INPUT);
      pinMode(41,OUTPUT);
      pinMode(40,OUTPUT);
      digitalWrite(41,LOW);
      digitalWrite(40,HIGH);
      break;
    }
    case 1: {
      pinMode(40,INPUT);
      pinMode(42,OUTPUT);
      pinMode(41,OUTPUT);
      digitalWrite(42,LOW);
      digitalWrite(41,HIGH);
      break;
    }
    case 2: {
      pinMode(41,INPUT);
      pinMode(42,OUTPUT);
      pinMode(40,OUTPUT);
      digitalWrite(42,LOW);
      digitalWrite(40,HIGH);
      break;
    }
    case 3: {
      pinMode(42,INPUT);
      pinMode(40,OUTPUT);
      pinMode(41,OUTPUT);
      digitalWrite(40,LOW);
      digitalWrite(41,HIGH);
      break;
    }
    case 4: {
      pinMode(40,INPUT);
      pinMode(41,OUTPUT);
      pinMode(42,OUTPUT);
      digitalWrite(41,LOW);
      digitalWrite(42,HIGH);
      break;
    }
    default: {
      pinMode(40,INPUT);
      pinMode(41,INPUT);
      pinMode(42,INPUT);
    }
  }
}

void setRadioStation(int freq) {
  if (freq>0) {
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

void checkTouch() {
  int freq = 0;
  bool t11 = (touchRead(11)>TOUCH_SENSITIVITY);
  bool t12 = (touchRead(12)>TOUCH_SENSITIVITY);
  bool t13 = (touchRead(13)>TOUCH_SENSITIVITY);
  bool t14 = (touchRead(14)>TOUCH_SENSITIVITY);
  bool t7 = (digitalRead(7)==HIGH);
  if (t11 && t12 && t14) {
    //Easter egg mode when three buttons are pressed AND the fine tuning dial is at zero position
    easterEggMode = (analogRead(2)<10);
  }
  if (t11 && (!t12)) {
    currentPreset = 3;
    easterEggMode = false;
    freq = getFrequency(9,false);
    Debug("Preset 3: ");
    Debugln(freq);
    showLED(3);
  };
  if (t12 && (!t11)) {
    currentPreset = 2;
    easterEggMode = false;
    freq = getFrequency(3,false);
    Debug("Preset 2: ");
    Debugln(freq);
    showLED(2);
  };
  if (t14 && (!t11)) {
    currentPreset = 1;
    easterEggMode = false;
    freq = getFrequency(8,false);
    Debug("Preset 1: ");
    Debugln(freq);
    showLED(1);
  };
  if (t11 && t12 && (!t14)) {
    currentPreset = 4;
    easterEggMode = false;
    freq = getFrequency(10,false);
    Debug("Preset 4: ");
    Debugln(freq);
    showLED(4);
  }
  if (t13) {
    currentPreset = 0;
    easterEggMode = false;
    freq = getFrequency(1,true);
    Debug("Preset MAN: ");
    Debug(freq); //FM tuning
    Debug(" / ");
    Debug(map(analogRead(2),0,4095,870,1080)); //FM fine tuning
    if (t7) { //FM Lock
      Debugln(" (on)");
    } else {
      Debugln(" (off)");
    }
    showLED(0);
  };
  if (t7 && (currentPreset==0)) {
    freq = getFrequency(1,true);
  }
  if (freq!=0) {
    setRadioStation(freq);
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
        rgbLedWrite(RGB_BUILTIN,0,0,0); //LED off
        setupAudio();
      }
    }
  }
  checkSensors();
  checkEasterEggMode();
  ElegantOTA.loop();
  vTaskDelay(1);
}
