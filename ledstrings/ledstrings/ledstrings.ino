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
char ssidAP[] = SECRET_SSID_AP;
char passAP[] = SECRET_PASS_AP;

int status = WL_IDLE_STATUS;

int timeZoneOffsetHours = 1;
WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient timeClient(Udp);

WiFiServer server(80);
WiFiClient client;

byte opsStatus = LOW;

time_t startTime;
time_t statusUpdateTime;
time_t connectTime;
time_t currentTime;
int reconnectLog = 0;

//The RTC is not very reliable, we need to adjust the clock...
time_t initCorrectTime = 0 ;
time_t newCorrectTime = 0 ;
time_t currentDeviatedTime = 0;
int timeDeviationPerHour = 88; //the calculated time deviation per hour, 88 seems to be a somewhat correct number
int minutesPast = 0;

RTCTime currentRTCTime;
RTCTime riseTime;
RTCTime setTime;

#define PAGE_HOME 0
#define PAGE_SETTIME 1
int requestedPage = PAGE_HOME;

//If network is not available, we fall back to an access point
boolean networkAvailable = false;

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
#define MODE_FIRE 7
#define MODE_METEOR 8
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
  digitalWrite(LED_BUILTIN,HIGH);
  int maxTry = 0;
  networkAvailable = false;
  while ((status != WL_CONNECTED) && (maxTry < 2)) {
    #ifdef SERIAL_ON
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    #endif
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    maxTry++;

    // acknowledge that we have lift-off
    // wait 5 seconds for connection:
    // flash fast to notify we try to get a WiFi connection
    for (int i=0; i<10; i++) {
      digitalWrite(LED_BUILTIN,LOW);
      delay(250);
      digitalWrite(LED_BUILTIN,HIGH);
      delay(250);
    }
  }
  networkAvailable = true;
  digitalWrite(LED_BUILTIN,LOW);
}

void checkWiFiStatus() {
  if (networkAvailable) {
    status = WiFi.status();
    if (status!=WL_CONNECTED) {
      #ifdef SERIAL_ON
      Serial.println("Lost internet connection to SSID!");
      Serial.println("Trying to reconnect...");
      #endif

      //Trying to reconnection to the WiFI
      setupWiFi();

      if (networkAvailable) {
        reconnectLog++;
      } else {
        #ifdef SERIAL_ON
        Serial.println("Connection could not be restored, try later...");
        #endif
        networkAvailable = true; //It should available, so we retry later (we don't create a access point at retry!)
      }
    }
  }
}

void setupWiFiAccessPoint() {
  // attempt to connect to WiFi network:
  digitalWrite(LED_BUILTIN,HIGH);

  // print the network name (SSID);
  Serial.print("Creating access point named: ");
  Serial.println(ssidAP);

  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(ssidAP, passAP);
  if (status != WL_AP_LISTENING) {
    #ifdef SERIAL_ON
    Serial.println("Creating access point failed");
    #endif
    // don't continue
    while (true);
  }

  // wait 5 seconds for connection:
  delay(5000);
  digitalWrite(LED_BUILTIN,LOW);

}

void getNetworkTime(boolean initial) {
  //Only attempt to call the server when we have a network available
  if (networkAvailable) {
    #ifdef SERIAL_ON
    Serial.println("Retrieving time from NTP server");
    #endif
    timeClient.begin();
    boolean succes = false;
    if (initial) {
      //Initialy we _need_ a correct time, so keep trying!
      int maxTry = 0;
      while ((!succes) && (maxTry<10)) {
        succes = timeClient.update();
        delay(1000);
        timeClient.forceUpdate();
        maxTry++;
        #ifdef SERIAL_ON
        Serial.println("Timeserver didn't respond - retry");
        #endif
      }
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

      recalculateDeviation(unixTime);
      RTCTime timeToSet = RTCTime(unixTime);
      RTC.setTime(timeToSet);

      // Retrieve the date and time from the RTC and print them
      RTC.getTime(currentRTCTime);
      #ifdef SERIAL_ON
      Serial.println("The RTC was just set to: " + String(currentRTCTime));
      #endif

      timeClient.end(); //End the timeclient, as we will only call this routine ones a day
    }
  }
}

void recalculateDeviation(time_t retrievedTime) {
  if (initCorrectTime==0) {
    initCorrectTime = retrievedTime;
  } else {
    newCorrectTime = retrievedTime;
    RTC.getTime(currentRTCTime);
    currentDeviatedTime = currentRTCTime.getUnixTime();
    //The difference between the init en new correct time is the deviation period in seconds
    //The difference between the currentDeviatedTime and the new correct time is the deviation in seconds
    //So the quotient of these two is the deviation in seconds, multiplied with 3600, we get the deviation in seconds per hour
    //This is the amount we have to substract from the current time every hour
    //The time deviation is the relevant deviation AFTER this correction, so we need to take the original deviation into account!
    int interval = newCorrectTime - initCorrectTime;
    if (interval>3600) { //Interval should be at least an hour, or the calculation will have large errors due to round-offs
      int newDeviation = timeDeviationPerHour + (((currentDeviatedTime - newCorrectTime) * 3600) / interval);
      #ifdef SERIAL_ON
      Serial.print("Current deviation: ");
      Serial.println(timeDeviationPerHour);
      Serial.print("Current deviated time: ");
      Serial.println(currentDeviatedTime);
      Serial.print("New correct time: ");
      Serial.println(newCorrectTime);
      Serial.print("init correct time: ");
      Serial.println(initCorrectTime);
      Serial.print("Deviation: ");
      Serial.println(newDeviation);
      #endif
      if (abs(newDeviation)<240) { //Sanity check: deviation should not be more than 4 minutes per hour!
        timeDeviationPerHour = round(newDeviation);
      }
    }
  }
}

void updateLedStatus() {
  RTC.getTime(currentRTCTime);
  time_t utcTime = currentRTCTime.getUnixTime() - (timeZoneOffsetHours * 3600); //We want UTC, so we need tot go back in time...
  SunRise sr;
  sr.calculate(52.155170, 5.387200, utcTime);
  riseTime = RTCTime(sr.riseTime + (timeZoneOffsetHours * 3600));
  setTime = RTCTime(sr.setTime + (timeZoneOffsetHours * 3600));
  #ifdef SERIAL_ON
  Serial.println("Current time: " + String(currentRTCTime));
  Serial.println("Sunrise at: " + String(riseTime));
  Serial.println("Sunset at: " + String(setTime));
  #endif
  int currentMinutes = currentRTCTime.getHour()*60+currentRTCTime.getMinutes();
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

  //Try a connection to the WiFI
  setupWiFi();
  //If this fails, create an access point
  if (!networkAvailable) {
    setupWiFiAccessPoint();
  }

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

void printHtmlHead() {
  // send the HTTP response body
  client.print(F("<!DOCTYPE HTML>\r\n"));
  client.print(F("<html lang=\"en\" data-bs-theme=\"dark\">\r\n"));
  client.print(F("<head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
  client.print(F("<title>LED string control</title>"));
  client.print(F("<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-T3c6CoIi6uLrA9TneNEoa7RxnatzjcDSCmG1MXxSR1GAsXEV/Dwwykc2MPK8M2HN\" crossorigin=\"anonymous\"></head>\r\n"));
  client.print(F("<body><div class=\"container\">"));
}

void printForm() {
  client.print(F("<form action=\"/\">"));
}

void printFooter() {
  client.print(F("</div></form></div></body></html>\r\n"));
}

void sendTimepageBody() {

  printHtmlHead();

  client.print(F("<h3><span class=\"badge\">Reconnect: "));
  client.print(reconnectLog);
  client.print(F("</span></h3>"));

  printForm();
  printDiv(false,true);
  client.print(F("<label class=\"form-label\" for=\"time\">Time</label>"));
  client.print(F("<input type=\"time\" class=\"form-control\" name=\"time\" id=\"time\">"));
  printDiv(true,true);
  client.print(F("<label class=\"form-label\" for=\"date\">Date</label>"));
  client.print(F("<input type=\"date\" class=\"form-control\" name=\"date\" id=\"date\">"));
  printDiv(true,false);
  client.print(F("<button type=\"submit\" class=\"btn btn-warning\">Submit</button>"));
  client.print(F("<script>var date = new Date(); document.getElementById('date').valueAsDate = date; document.getElementById('time').value = date.toTimeString().substring(0,5);</script>\r\n"));
  printFooter();

}

void sendHomepageBody() {

  printHtmlHead();

  client.print(F("<h3><span class=\"badge\"><a href=\"/settime\">"));
  client.print(String(currentRTCTime));
  client.print(F("</a> ("));
  if (timeDeviationPerHour>0) {
    client.print("+");
  }
  client.print(timeDeviationPerHour);
  client.print(F(")</span></h3>"));
  client.print(F("<h3><span class=\"badge\">&#x1F305; "));
  client.print(String(riseTime).substring(11,16));
  client.print(F(" - &#x1F307; "));
  client.print(String(setTime).substring(11,16));
  client.print(F(" - "));
  client.print(FACTSTR[factStatus]);
  client.print(F("</span></h3>\r\n"));

  printForm();
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
  printOption("mode","Fire",MODE_FIRE,ledMode);
  printOption("mode","Meteor",MODE_METEOR,ledMode);
  printDiv(true,true);
  client.print(F("<label class=\"form-label\" for=\"brightness\">Brightness</label>"));
  client.print(F("<input type=\"range\" class=\"form-range\" name=\"brightness\" id=\"brightness\" min=\"0\" max=\"255\" value=\""));
  client.print(maxBrightness);
  client.print("\">");
  printDiv(true,true);
  client.print(F("<button type=\"submit\" class=\"btn btn-warning\">Submit</button>"));
  client.print(F("<a href=\"/\" class=\"btn btn-info\" role=\"button\">Refresh</a>"));
  printFooter();
}

Month intToMonth(int m) {
  switch(m) {
    case 1: return Month::JANUARY;
    case 2: return Month::FEBRUARY;
    case 3: return Month::MARCH;
    case 4: return Month::APRIL;
    case 5: return Month::MAY;
    case 6: return Month::JUNE;
    case 7: return Month::JULY;
    case 8: return Month::AUGUST;
    case 9: return Month::SEPTEMBER;
    case 10: return Month::OCTOBER;
    case 11: return Month::NOVEMBER;
    case 12: return Month::DECEMBER;
  }
}

void checkWebClient() {
  connectTime = currentTime;
  // listen for incoming clients
  client = server.available();

  if (client) {
    #ifdef SERIAL_ON
    Serial.println(F("New client"));
    #endif
    // an http request ends with a blank line
    bool currentLineIsBlank = true;

    while (client.connected()) {
      // currentTime = millis(); Set currentTime only ones (in main loop)
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
          if (requestedPage==PAGE_SETTIME) {
            sendTimepageBody();
          } else {
            sendHomepageBody();
          }

          break;
        } else {
          //Handle request
          RTC.getTime(currentRTCTime);

          if (req.startsWith("GET /settime")) {
            requestedPage = PAGE_SETTIME;
          } else if (req.startsWith("GET /?")) {
            requestedPage = PAGE_HOME;
            boolean changed = false;
            boolean timeChanged = false;
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
            pos = req.indexOf("time");
            RTCTime newRTCTime;
            if (pos>0) {
              RTC.getTime(newRTCTime);
              currentDeviatedTime = newRTCTime.getUnixTime();
              #ifdef SERIAL_ON
              Serial.println("Hour: ["+req.substring(pos+5,pos+7)+"] Minutes: ["+req.substring(pos+10,pos+12)+"]");
              #endif
              newRTCTime.setHour(req.substring(pos+5,pos+7).toInt());
              newRTCTime.setMinute(req.substring(pos+10,pos+12).toInt());
              timeChanged = true;
            }
            pos = req.indexOf("date");
            if (pos>0) {
              newRTCTime.setYear(req.substring(pos+5,pos+9).toInt());
              newRTCTime.setMonthOfYear(intToMonth(req.substring(pos+10,pos+12).toInt()));
              newRTCTime.setDayOfMonth(req.substring(pos+13,pos+15).toInt());
              timeChanged = true;
            }
            if (timeChanged) {
              recalculateDeviation(newRTCTime.getUnixTime());
              RTC.setTime(newRTCTime);
              updateLedStatus();
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

void checkInternalLed() {
  // currentTime = millis(); Set currentTime only ones (in main loop)
  if ((currentTime - startTime) > 1000) {
    opsStatus = !opsStatus;
    digitalWrite(LED_BUILTIN,opsStatus);
    startTime = currentTime;
    checkWiFiStatus(); //Do we still have an internet connection?
    checkWebClient(); //We only check the webclient ones a second
  }
}

//Ones every minute, the LED status will be updated to the current situation
//Ones every hour, we will adjust the RTC
void checkStatusUpdate() {
  // currentTime = millis(); Set currentTime only ones (in main loop)
  if ((currentTime - statusUpdateTime) > 60000) {
    updateLedStatus();
    statusUpdateTime = currentTime;
    minutesPast++;
    if (minutesPast>=60) {
      minutesPast=0;
      RTC.getTime(currentRTCTime);
      currentDeviatedTime = currentRTCTime.getUnixTime() - timeDeviationPerHour;
      currentRTCTime.setUnixTime(currentDeviatedTime);
      RTC.setTime(currentRTCTime);
    }
  }
}

void loop() {
  currentTime = millis(); //Set currentTime only ones!
  checkInternalLed();
  checkStatusUpdate();
  updateLEDs();
}
