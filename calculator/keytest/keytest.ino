#include "Pinconfig.h"

void setup() {
  Pinconfig.init();
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,LOW);
  Serial.begin(9600); //For testing only
  Serial.println("=============================");
  Serial.println("Start testing...");
}

//Current example requires for slider:
//- ESP32 pin 14 connected to calculator pin 5 via 1k resistor
//- ESP32 pin 16 connected to calculator pin 1
//- ESP32 pin 4 connected to calculator pin 2
//- ESP32 pin 2 connected to calculator pin 3
//- ESP32 pin 25 connected to calculator pin 4

//Current example requires for keys 0,1,2,7:
// - ESP32 pin 0 connected to calculator pin 8 via 1k resistor
// - ESP32 pin 32 connected to calculator pin 20 (key 7)
// - ESP32 pin 33 connected to calculator pin 22 (key 2)
// - ESP32 pin 26 connected to calculator pin 24 (key 1)
// - ESP32 pin 27 connected to calculator pin 25 (key 0)

void delayCheck() {
  KeyRaw keys;
  if (Pinconfig.testKeysDelay(1000,keys)) {
    Serial.print("Key pressed: ");
    Serial.print(keys.key1); Serial.print("-");
    Serial.print(keys.key2); Serial.print("-");
    Serial.print(keys.key3); Serial.print("-");
    Serial.print(keys.key4); Serial.print("-");
    Serial.println(".");
  }
}

void loop() {
  digitalWrite(PIN_LED,HIGH);
  int decimals = Pinconfig.testDecimals();
  Serial.print("Decimal slider value: ");
  Serial.print(decimals);
  Serial.println(".");
  delayCheck();
  digitalWrite(PIN_LED,LOW);
  delayCheck();
}
