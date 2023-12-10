/*
 * LED Strings
 *
 * Application that does:
 *
 * - Fetching the time from an NTP server (ones a day)
 * - Starting a webserver, to control the LEDS
 * - controlling a WS2813 LED string
 *
 */

//Uncommit if you want to see serial messages
#define SERIAL_ON

#include <WiFiS3.h>
#include "RTC.h"
#include <NTPClient.h>

// Sensitive data in secrets.h
#include "secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;

WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient timeClient(Udp);

WiFiServer server(80);

boolean ledsOn = true;
byte ledsStatus = LOW;

time_t startTime;
time_t connectTime;
time_t currentTime;

void setup() {
  resetLedTimer();

  //Initialize serial and wait for port to open:
  #ifdef SERIAL_ON
  Serial.begin(9600);

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

  RTC.begin();
  #ifdef SERIAL_ON
  Serial.println("Retrieving time from NTP server");
  #endif
  timeClient.begin();
  timeClient.update();

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

  //Start the webserver
  server.begin();
  #ifdef SERIAL_ON
  printWifiStatus();
  #endif
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
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println();                     // the separator between HTTP header and body
}

void sendHomepageBody(WiFiClient& client) {
  // send the HTTP response body
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  client.println("</head>");

  RTCTime currentTime;
  RTC.getTime(currentTime);
  client.print("<p style=\"font-size:7vw;\">Time: ");
  client.print(String(currentTime));
  client.println("</p>");

  client.println("<p style=\"font-size:7vw;\">Click <a href=\"/H\">here</a> turn the LED on<br></p>");
  client.println("<p style=\"font-size:7vw;\">Click <a href=\"/L\">here</a> turn the LED off<br></p>");

  client.println("</html>");
}

void checkWebClient() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    // read the HTTP request header line by line
    Serial.println("new client");
    while (client.connected()) {
      delayMicroseconds(10);
      if (client.available()) {
        String HTTP_header = client.readStringUntil('\n');  // read the header line of HTTP request

        if (HTTP_header.equals("\r"))  // the end of HTTP request
          break;

        #ifdef SERIAL_ON
        Serial.print("<< ");
        Serial.println(HTTP_header);  // print HTTP request to Serial Monitor
        #endif
      }
    }

    sendHttpHeader(client);
    sendHomepageBody(client);
    client.flush();
    // give the web browser time to receive the data
    delay(10);
    // close the connection:
    client.stop();
  }
}

void checkWebClient2() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port

    connectTime = millis();
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      delayMicroseconds(10);                // This is required for the Arduino Nano RP2040 Connect - otherwise it will loop so fast that SPI will never be served.
      currentTime = millis();
      if ((currentTime - connectTime) > 5000) {
        Serial.println("Connection timeout");
        client.stop();
      }
      if (client.available()) {             // if there's bytes to read from the client,
        connectTime = millis();
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out to the serial monitor
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
          ledsOn = true;
        }
        if (currentLine.endsWith("GET /L")) {
          ledsOn = false;
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void checkWebClient3()
{
  // listen for incoming clients
  WiFiClient client = server.available();

  if (client)
  {
    Serial.println(F("New client"));
    // an http request ends with a blank line
    bool currentLineIsBlank = true;

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank)
        {
          Serial.println(F("Sending response"));

          // send a standard http response header
          // use \r\n instead of many println statements to speedup data send
          client.print(
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"  // the connection will be closed after completion of the response
            "Refresh: 20\r\n"        // refresh the page automatically every 20 sec
            "\r\n");
          client.print(F("<!DOCTYPE HTML>\r\n"));
          client.print(F("<html>\r\n"));
          client.print(F("<h1>Hello World from "));
          client.print(F("!</h1>\r\n"));
          client.print(F("Requests received: "));
          client.print(F("<br>\r\n"));
          client.print(F("Analog input A0: "));
          client.print(F("<br>\r\n"));
          client.print(F("</html>\r\n"));
          break;
        }

        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }

    // give the web browser time to receive the data
    delay(10);

    // close the connection:
    client.stop();
    Serial.println(F("Client disconnected"));
  }
}

void resetLedTimer() {
  startTime = millis();
}

void checkLedStrings() {
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

void loop() {
  checkWebClient3();
  checkLedStrings();
}
