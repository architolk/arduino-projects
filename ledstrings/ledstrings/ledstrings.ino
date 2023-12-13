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

boolean ledsOn = true;
byte ledsStatus = LOW;

time_t startTime;
time_t statusUpdateTime;
time_t connectTime;
time_t currentTime;

// We've got three states:
// 1. Leds on: when the sun has set and it's before the sleep time OR when its after the wake up time
// 2. Leds dimmed: when the sun has set and it's after the sleep time and before the wake up time
// 3. Leds off: when the sun has risen
#define LEDS_DIMMED 1
#define LEDS_ON_DAWN 2
#define LEDS_ON 3
#define LEDS_OFF 0
int sleepTime = 23*60 + 0; //Time to switch off all lights
int wakeupTime = 7*60 + 30; //Time to switch on the lights
byte ledStatus = LEDS_ON;

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

void getNetworkTime() {
  #ifdef SERIAL_ON
  Serial.println("Retrieving time from NTP server");
  #endif
  timeClient.begin();
  timeClient.update();

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

void updateLedStatus() {
  RTCTime currentTime;
  RTC.getTime(currentTime);
  time_t utcTime = currentTime.getUnixTime() - (timeZoneOffsetHours * 3600); //We want UTC, so we need tot go back in time...
  SunRise sr;
  sr.calculate(52.155170, 5.387200, utcTime);
  RTCTime riseTime = RTCTime(sr.riseTime + (timeZoneOffsetHours * 3600));
  RTCTime setTime = RTCTime(sr.setTime + (timeZoneOffsetHours * 3600));
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
      ledStatus = LEDS_ON_DAWN;
    } else {
      #ifdef SERIAL_ON
      Serial.println("It's daytime (sun has risen)");
      #endif
      ledStatus = LEDS_OFF;
    }
  } else {
    Serial.println("It's night (sun has set)");
    if ((currentMinutes>=sleepTime) || (currentMinutes<wakeupTime)) {
      #ifdef SERIAL_ON
      Serial.println("Time to sleep - Leds are dimmed");
      #endif
      ledStatus = LEDS_DIMMED;
    } else {
      #ifdef SERIAL_ON
      Serial.println("It's dark - Leds are on");
      #endif
      if (ledStatus!=LEDS_ON) {
        //ledStatus has changed, so we need to update the RTC via the network
        //This might result in another change, but as LEDS_ON and LEDS_ON_DAWN are actually the same, this will not happen
        getNetworkTime();
      }
      ledStatus = LEDS_ON;
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
  getNetworkTime();

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

void sendHttpHeader(WiFiClient& client) {
  // send the HTTP response
  // send the HTTP response header
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"  // the connection will be closed after completion of the response
    "\r\n");
}

void sendHomepageBody(WiFiClient& client) {
  // send the HTTP response body
  client.print(F("<!DOCTYPE HTML>\r\n"));
  client.print(F("<html>\r\n"));

  RTCTime currentTime;
  RTC.getTime(currentTime);
  client.print(F("<p style=\"font-size:7vw;\">Time: "));
  client.print(String(currentTime));
  client.print(F("</p>\r\n"));

  client.print(F("<p style=\"font-size:7vw;\">Click <a href=\"/H\">here</a> turn the LED on<br></p>\r\n"));
  client.print(F("<p style=\"font-size:7vw;\">Click <a href=\"/L\">here</a> turn the LED off<br></p>\r\n"));

  client.print(F("</html>\r\n"));
}

void checkWebClient() {
  connectTime = millis();
  // listen for incoming clients
  WiFiClient client = server.available();

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

          sendHttpHeader(client);
          sendHomepageBody(client);

          break;
        } else {
          //Handle request
          if (req.startsWith("GET /H")) {
            ledsOn = true;
          }
          if (req.startsWith("GET /L")) {
            ledsOn = false;
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
    ledsStatus = !ledsStatus;
    if (ledsOn) {
      digitalWrite(LED_BUILTIN,ledsStatus);
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
  checkLedString();
}
