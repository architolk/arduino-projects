#include "Pinconfig.h"

void setup() {
  Pinconfig.init();
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,LOW);
  Serial.begin(9600); //For testing only
  Serial.println("=============================");
  Serial.println("Start testing...");
}

//Current example requires:
//- ESP32 pin 14 connected to calculator pin 5 via 10k resistor
//- ESP32 pin 16 connected to calculator pin 1
//- ESP32 pin 4 connected to calculator pin 2
//- ESP32 pin 2 connected to calculator pin 3
//- ESP32 pin 25 connected to calculator pin 4

void loop() {
  digitalWrite(PIN_LED,HIGH);
  int decimals = Pinconfig.testDecimals();
  Serial.print("Decimal slider value: ");
  Serial.print(decimals);
  Serial.println(".");
  delay(1000);
  digitalWrite(PIN_LED,LOW);
  delay(1000); //Wait 4 seconds
}
