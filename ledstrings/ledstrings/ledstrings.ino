/*
 * LED Strings
 *
 * Application that does:
 *
 * - Fetching the time from an NTP server (ones a day)
 * - Starting a webserver, to control the LEDS
 * - controlling a WS2813 LED string
 *
 * REMARK
 * Using the Arduino UNO R4 Wifi
 * FastLED 3.6.0 doesn't work - you need the master branch
 * AND you need https://github.com/FastLED/FastLED/pull/1554
 */

//Uncommit if you want to see serial messages
//#define SERIAL_ON

#include <WiFiS3.h>
#include "RTC.h"
#include <NTPClient.h>
#include <SunRise.h>

// Sensitive data in secrets.h
#include "secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;

int timeZoneOffsetHours = 1;
WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient timeClient(Udp);

WiFiServer server(80);
WiFiClient client;

boolean ledsOn = true;
byte opsStatus = LOW;

time_t startTime;
time_t statusUpdateTime;
time_t connectTime;
time_t currentTime;

RTCTime riseTime;
RTCTime setTime;

//Global default maxBrightness
int maxBrightness = 100;

// We've got three states:
// 1. Leds on: when the sun has set and it's before the sleep time OR when its after the wake up time
// 2. Leds dimmed: when the sun has set and it's after the sleep time and before the wake up time
// 3. Leds off: when the sun has risen
#define LEDS_DIMMED 1
#define LEDS_ON 3
#define LEDS_OFF 0
int sleepTime = 23*60 + 0; //Time to switch off all lights (23:00)
int wakeupTime = 7*60 + 30; //Time to switch on the lights (7:30)
int ledStatus = LEDS_ON;

#define MODE_DISCRETE 1
#define MODE_CHRISTMAS 2
#define MODE_NL 3
#define MODE_CHASER 4
#define MODE_RAINBOW 5
#define MODE_STAIRCASE 6
int ledMode = MODE_DISCRETE;

#define FACT_DARK 0
#define FACT_NIGHT 1
#define FACT_DAY 2
#define FACT_DAWN 3
int factStatus = FACT_DARK;

const char STR_DARK[] PROGMEM = "It's dark";
const char STR_NIGHT[] PROGMEM = "It's night";
const char STR_DAY[] PROGMEM = "It's daytime";
const char STR_DAWN[] PROGMEM = "It's dawn";
const char * const FACTSTR[4] = {STR_DARK,STR_NIGHT,STR_DAY,STR_DAWN};

void setupWiFi() {
  #ifdef SERIAL_ON
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }
  #endif

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    #ifdef SERIAL_ON
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    #endif
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
}

void getNetworkTime(boolean initial) {
  #ifdef SERIAL_ON
  Serial.println("Retrieving time from NTP server");
  #endif
  timeClient.begin();
  boolean succes = false;
  if (initial) {
    //Initialy we _need_ a correct time, so keep trying!
    while (!timeClient.update()) {
      delay(1000);
      timeClient.forceUpdate();
      #ifdef SERIAL_ON
      Serial.println("Timeserver didn't respond - retry");
      #endif
    }
    succes = true; //We won't get here, if we had no succes, so succes is implied
  } else {
    succes = timeClient.update();
    #ifdef SERIAL_ON
    if (!succes) {
      Serial.println("Timeserver didn't respond - ignore");
    }
    #endif
  }

  //Time client might return false, (non-responding time server), so we just ignore that situation and continue with our original time...
  if (succes) {
    time_t utcTime = timeClient.getEpochTime();
    time_t unixTime = utcTime + (timeZoneOffsetHours * 3600);

    #ifdef SERIAL_ON
    Serial.print("Unix time = ");
    Serial.println(unixTime);
    #endif
    RTCTime timeToSet = RTCTime(unixTime);
    RTC.setTime(timeToSet);

    // Retrieve the date and time from the RTC and print them
    RTCTime currentTime;
    RTC.getTime(currentTime);
    #ifdef SERIAL_ON
    Serial.println("The RTC was just set to: " + String(currentTime));
    #endif

    timeClient.end(); //End the timeclient, as we will only call this routine ones a day
  }
}

void updateLedStatus() {
  RTCTime currentTime;
  RTC.getTime(currentTime);
  time_t utcTime = currentTime.getUnixTime() - (timeZoneOffsetHours * 3600); //We want UTC, so we need tot go back in time...
  SunRise sr;
  sr.calculate(52.155170, 5.387200, utcTime);
  riseTime = RTCTime(sr.riseTime + (timeZoneOffsetHours * 3600));
  setTime = RTCTime(sr.setTime + (timeZoneOffsetHours * 3600));
  #ifdef SERIAL_ON
  Serial.println("Sunrise at: " + String(riseTime));
  Serial.println("Sunset at: " + String(setTime));
  #endif
  int currentMinutes = currentTime.getHour()*60+currentTime.getMinutes();
  if (sr.isVisible) {
    int dawnMinutes = setTime.getHour()*60+setTime.getMinutes()-60; //Sixty minutes before sunset is considered dawn
    if (currentMinutes>=dawnMinutes) {
      #ifdef SERIAL_ON
      Serial.println("It's dawn - Leds are on");
      #endif
      if (factStatus!=FACT_DAWN) {
        factStatus = FACT_DAWN;
        if (ledStatus!=LEDS_ON) {
          ledStatus = LEDS_ON;
          initLEDs();
        }
      }
    } else {
      #ifdef SERIAL_ON
      Serial.println("It's daytime (sun has risen)");
      #endif
      if (factStatus!=FACT_DAY) {
        factStatus = FACT_DAY;
        if (ledStatus!=LEDS_OFF) {
          ledStatus = LEDS_OFF;
          initLEDs();
        }
      }
    }
  } else {
    Serial.println("It's night (sun has set)");
    if ((currentMinutes>=sleepTime) || (currentMinutes<wakeupTime)) {
      #ifdef SERIAL_ON
      Serial.println("Time to sleep - Leds are dimmed");
      #endif
      if (factStatus!=FACT_NIGHT) {
        factStatus = FACT_NIGHT;
        if (ledStatus!=LEDS_DIMMED) {
          ledStatus = LEDS_DIMMED;
          initLEDs();
        }
      }
    } else {
      #ifdef SERIAL_ON
      Serial.println("It's dark - Leds are on");
      #endif
      if (factStatus!=FACT_DARK) {
        //ledStatus has changed, so we need to update the RTC via the network
        //This might result in another change, but as FACT_DARK and FACT_DAWN both result in LEDS_ON, this will not happen
        getNetworkTime(false);
        factStatus = FACT_DARK;
        if (ledStatus!=LEDS_ON) {
          ledStatus = LEDS_ON;
          initLEDs();
        }
      }
    }
  }
}

void setup() {
  //Initialize serial and wait for port to open:
  #ifdef SERIAL_ON
  Serial.begin(9600);
  #endif

  setupWiFi();

  RTC.begin(); //Start the real time clock
  getNetworkTime(true);

  updateLedStatus();

  //Start the webserver
  server.begin();

  //Print some status info
  #ifdef SERIAL_ON
  printWifiStatus();
  #endif

  //Setup for the LEDs (init of FastLED etc)
  setupLEDs();

  resetLedTimer();
  resetStatusUpdateTimer();
}

void printWifiStatus() {
  // print your board's IP address:
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // print the received signal strength:
  Serial.print("signal strength (RSSI):");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  Serial.println("Webserver started, port 80");
}

void sendHttpHeader() {
  // send the HTTP response
  // send the HTTP response header
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"  // the connection will be closed after completion of the response
    "\r\n");
}

void printDiv(boolean close, boolean flex) {
  if (close) {
    client.print(F("</div>\r\n"));
  }
  client.print(F("<div class=\"mb-3"));
  if (flex) {
    client.print(F(" d-flex\" style=\"gap:10px"));
  }
  client.print("\">");
}

void printOption(char* name, char* label, int value, int current) {
  client.print(F("<input type=\"radio\" class=\"btn-check\" name=\""));
  client.print(name);
  client.print(F("\" id=\""));
  client.print(name); client.print(value);
  client.print(F("\" value=\""));
  client.print(value);
  client.print("\"");
  if (value==current) {
    client.print(F(" checked"));
  }
  client.print(F("><label class=\"btn\" for=\""));
  client.print(name); client.print(value);
  client.print("\">");
  client.print(label);
  client.print(F("</label>"));
}

void sendHomepageBody() {
  // send the HTTP response body
  client.print(F("<!DOCTYPE HTML>\r\n"));
  client.print(F("<html lang=\"en\" data-bs-theme=\"dark\">\r\n"));
  client.print(F("<head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
  client.print(F("<title>LED string control</title>"));
  client.print(F("<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-T3c6CoIi6uLrA9TneNEoa7RxnatzjcDSCmG1MXxSR1GAsXEV/Dwwykc2MPK8M2HN\" crossorigin=\"anonymous\"></head>\r\n"));
  client.print(F("<body><div class=\"container\">"));

  RTCTime currentTime;
  RTC.getTime(currentTime);

  client.print(F("<h3><span class=\"badge\">"));
  client.print(String(currentTime));
  client.print(F("</span></h3>"));
  client.print(F("<h3><span class=\"badge\">&#x1F305; "));
  client.print(String(riseTime).substring(11,16));
  client.print(F(" - &#x1F307; "));
  client.print(String(setTime).substring(11,16));
  client.print(F(" - "));
  client.print(FACTSTR[factStatus]);
  client.print(F("</span></h3>\r\n"));

  client.print(F("<form>"));
  printDiv(false,false);
  printOption("status","LEDs on",LEDS_ON,ledStatus);
  printOption("status","LEDs dimmed",LEDS_DIMMED,ledStatus);
  printOption("status","LEDs off",LEDS_OFF,ledStatus);
  printDiv(true,false);
  printOption("mode","Discrete",MODE_DISCRETE,ledMode);
  printOption("mode","Christmas",MODE_CHRISTMAS,ledMode);
  printOption("mode","NL flag",MODE_NL,ledMode);
  printOption("mode","Chaser",MODE_CHASER,ledMode);
  printOption("mode","Rainbow",MODE_RAINBOW,ledMode);
  printOption("mode","Staircase",MODE_STAIRCASE,ledMode);
  printDiv(true,true);
  client.print(F("<label class=\"form-label\" for=\"brightness\">Brightness</label>"));
  client.print(F("<input type=\"range\" class=\"form-range\" name=\"brightness\" id=\"brightness\" min=\"0\" max=\"255\" value=\""));
  client.print(maxBrightness);
  client.print("\">");
  printDiv(true,true);
  client.print(F("<button type=\"submit\" class=\"btn btn-warning\">Submit</button>"));
  client.print(F("<a href=\"/\" class=\"btn btn-info\" role=\"button\">Refresh</a>"));
  client.print(F("</div></form></div></body></html>\r\n"));
}

void checkWebClient() {
  connectTime = millis();
  // listen for incoming clients
  client = server.available();

  if (client) {
    #ifdef SERIAL_ON
    Serial.println(F("New client"));
    #endif
    // an http request ends with a blank line
    bool currentLineIsBlank = true;

    while (client.connected()) {
      currentTime = millis();
      if ((currentTime - connectTime) > 5000) {
        #ifdef SERIAL_ON
        Serial.println("Connection timeout");
        #endif
        client.stop();
        return;
      }
      if (client.available()) {
        String req = client.readStringUntil('\r');
        client.readStringUntil('\n');
        #ifdef SERIAL_ON
        Serial.println(req);
        #endif
        connectTime = currentTime;

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (req.length()==0) {
          #ifdef SERIAL_ON
          Serial.println(F("Sending response"));
          #endif

          sendHttpHeader();
          sendHomepageBody();

          break;
        } else {
          //Handle request
          if (req.startsWith("GET /?")) {
            boolean changed = false;
            int pos = req.indexOf("status=");
            if (pos>0) {
              int newLedStatus = req.substring(pos+7,pos+8).toInt();
              if (newLedStatus!=ledStatus) {
                ledStatus = newLedStatus;
                changed = true;
              }
              #ifdef SERIAL_ON
              Serial.println(">>> Status: [" + req.substring(pos+7,pos+8) + "]");
              #endif
            }
            pos = req.indexOf("mode=");
            if (pos>0) {
              int newLedMode = req.substring(pos+5,pos+6).toInt();
              if (newLedMode!=ledMode) {
                ledMode = newLedMode;
                changed = true;
              }
              #ifdef SERIAL_ON
              Serial.println(">>> Mode: [" + req.substring(pos+5,pos+6) + "]");
              #endif
            }
            pos = req.indexOf("brightness=");
            if (pos>0) {
              int newBrightness = req.substring(pos+11,pos+14).toInt(); //Only works if brightness is the last element!
              if (newBrightness!=maxBrightness) {
                maxBrightness = newBrightness;
                changed = true;
              }
            }
            if (changed) {
              initLEDs(); //initialise the LEDs with the new settings
            }
          }
        }
      }
    }

    // give the web browser time to receive the data
    delay(10);

    // close the connection:
    client.stop();
    #ifdef SERIAL_ON
    Serial.println(F("Client disconnected"));
    #endif
  }
}

void resetLedTimer() {
  startTime = millis();
}

void resetStatusUpdateTimer() {
  statusUpdateTime = millis();
}

void checkInternalLed() {
  currentTime = millis();
  if ((currentTime - startTime) > 500) {
    opsStatus = !opsStatus;
    if (ledsOn) {
      digitalWrite(LED_BUILTIN,opsStatus);
    } else {
      digitalWrite(LED_BUILTIN,LOW);
    }
    resetLedTimer();
  }
}

//Ones every minute, the LED status will be updated to the current situation
void checkStatusUpdate() {
  currentTime = millis();
  if ((currentTime - statusUpdateTime) > 60000) {
    updateLedStatus();
    resetStatusUpdateTimer();
  }
}

void loop() {
  checkWebClient();
  checkInternalLed();
  checkStatusUpdate();
  updateLEDs();
}
