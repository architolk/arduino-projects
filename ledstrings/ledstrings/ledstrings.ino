// Include the RTC library
#include "RTC.h"

//Include the NTP library
#include <NTPClient.h>

#include <WiFiS3.h>

#include <WiFiUdp.h>

#include "secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

int wifiStatus = WL_IDLE_STATUS;
WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient timeClient(Udp);

WiFiServer server(80);

time_t startTime;
time_t currentTime;

//Serial on or off
#define SERIAL_ON 1

int led =  LED_BUILTIN;

#ifdef SERIAL_ON
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
#endif

void connectToWiFi(){
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    #ifdef SERIAL_ON
    Serial.println("Communication with WiFi module failed!");
    #endif
    // don't continue
    while (true);
  }

  #ifdef SERIAL_ON
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }
  #endif

  // attempt to connect to WiFi network:
  while (wifiStatus != WL_CONNECTED) {
    #ifdef SERIAL_ON
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    #endif
    // Connect to WPA/WPA2 network.
    wifiStatus = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  #ifdef SERIAL_ON
  Serial.println("Connected to WiFi");
  printWifiStatus();
  #endif
}

void setup(){
  #ifdef SERIAL_ON
  Serial.begin(9600);
  while (!Serial);
  #endif

  connectToWiFi();
  RTC.begin();
  #ifdef SERIAL_ON
  Serial.println("\nStarting connection to server...");
  #endif
  timeClient.begin();
  timeClient.update();

  // Get the current date and time from an NTP server and convert
  // it to UTC +2 by passing the time zone offset in hours.
  // You may change the time zone offset to your local one.
  auto timeZoneOffsetHours = 1;
  auto unixTime = timeClient.getEpochTime() + (timeZoneOffsetHours * 3600);
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

  // Start the webserver
  server.begin();
  #ifdef SERIAL_ON
  Serial.println("\nWebserver started at port 80");
  #endif

  resetLedTimer();
}

void checkWifiStatus() {
  // compare the previous status to the current status
  if (wifiStatus != WiFi.status()) {
    // it has changed update the variable
    wifiStatus = WiFi.status();
  }
}

void checkWifiClient() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    #ifdef SERIAL_ON
    Serial.println("new client");           // print a message out the serial port
    #endif
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      delayMicroseconds(10);                // This is required for the Arduino Nano RP2040 Connect - otherwise it will loop so fast that SPI will never be served.
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        #ifdef SERIAL_ON
        Serial.write(c);                    // print it out to the serial monitor
        #endif
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("<p style=\"font-size:7vw;\">Click <a href=\"/H\">here</a> turn the LED on<br></p>");
            client.print("<p style=\"font-size:7vw;\">Click <a href=\"/L\">here</a> turn the LED off<br></p>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(led, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(led, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    #ifdef SERIAL_ON
    Serial.println("client disconnected");
    #endif
  }
}

void resetLedTimer() {
  startTime = millis();
}

void checkLedStrings() {
  currentTime = millis();
  if ((currentTime - startTime) > 5000) {
    #ifdef SERIAL_ON
    Serial.println("Timer!");
    #endif
    resetLedTimer();
  }
}

void loop() {

  checkWifiStatus();
  checkWifiClient();
  checkLedStrings();

}
