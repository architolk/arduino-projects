#include "Pinconfig.h"

PINCONFIG Pinconfig;

void PINCONFIG::init() {
  //Setting input pins
  pinMode(PIN_KEYCHECK1, INPUT); //Cannot be set to pulldown, need external pulldown
  pinMode(PIN_KEYCHECK2, INPUT); //Cannot be set to pulldown, need external pulldown
  pinMode(PIN_KEYCHECK3, INPUT); //Cannot be set to pulldown, need external pulldown
  pinMode(PIN_KEYCHECK4, INPUT); //Cannot be set to pulldown, need external pulldown
  pinMode(PIN_KEYCHECK5, INPUT_PULLDOWN);
  pinMode(PIN_KEYCHECK6, INPUT_PULLDOWN);
  pinMode(PIN_KEYCHECK7, INPUT_PULLDOWN);
  pinMode(PIN_KEYCHECK8, INPUT_PULLDOWN);
  pinMode(PIN_SWCHECK1, INPUT_PULLDOWN);
  pinMode(PIN_SWCHECK2, INPUT_PULLDOWN);
  pinMode(PIN_SWCHECK3, INPUT_PULLDOWN);
  pinMode(PIN_SWCHECK4, INPUT_PULLDOWN);
  //Setting output pins and set to LOW
  pinMode(PIN_KEYPROBE1, OUTPUT);
  pinMode(PIN_KEYPROBE2, OUTPUT);
  pinMode(PIN_KEYPROBE3, OUTPUT);
  pinMode(PIN_KEYPROBE4, OUTPUT);
  pinMode(PIN_SWPROBE1, OUTPUT);
  pinMode(PIN_SWPROBE2, OUTPUT);
  pinMode(PIN_SWPROBE3, OUTPUT);
  digitalWrite(PIN_KEYPROBE1, LOW);
  digitalWrite(PIN_KEYPROBE2, LOW);
  digitalWrite(PIN_KEYPROBE3, LOW);
  digitalWrite(PIN_KEYPROBE4, LOW);
  digitalWrite(PIN_SWPROBE1, LOW);
  digitalWrite(PIN_SWPROBE2, LOW);
  digitalWrite(PIN_SWPROBE3, LOW);
}

int PINCONFIG::testDecimals() {
  digitalWrite(PIN_SWPROBE1, HIGH);
  delay(1); //For stability
  bool sw1 = digitalRead(PIN_SWCHECK1);
  bool sw2 = digitalRead(PIN_SWCHECK2);
  bool sw3 = digitalRead(PIN_SWCHECK3);
  bool sw4 = digitalRead(PIN_SWCHECK4);
  digitalWrite(PIN_SWPROBE1, LOW);
  delay(1); //For stability
  if (sw1) {
    return 4;
  } else if (sw2) {
    if (sw4) {
      return 3;
    } else {
      return 2;
    }
  } else if (sw3) {
    return -1;
  } else if (sw4) {
    return 1;
  } else {
    return 0;
  }
}

bool PINCONFIG::testPaperFeed() {
  digitalWrite(PIN_SWPROBE2, HIGH);
  delay(1); //For stability
  bool sw = digitalRead(PIN_SWCHECK3);
  digitalWrite(PIN_SWPROBE2, LOW);
  delay(1); //For stability
  return sw;
}

bool PINCONFIG::testRound() {
  digitalWrite(PIN_SWPROBE2, HIGH);
  delay(1); //For stability
  bool sw = digitalRead(PIN_SWCHECK1);
  digitalWrite(PIN_SWPROBE2, LOW);
  delay(1); //For stability
  return sw;
}

bool PINCONFIG::testK() {
  digitalWrite(PIN_SWPROBE2, HIGH);
  delay(1); //For stability
  bool sw = digitalRead(PIN_SWCHECK2);
  digitalWrite(PIN_SWPROBE2, LOW);
  delay(1); //For stability
  return sw;
}

int PINCONFIG::testItem() {
  digitalWrite(PIN_SWPROBE2, HIGH);
  delay(1); //For stability
  bool sw1 = digitalRead(PIN_SWCHECK3);
  bool sw2 = digitalRead(PIN_SWCHECK4);
  digitalWrite(PIN_SWPROBE2, LOW);
  delay(1); //For stability
  if (sw2) {
    if (sw1) {
      return 2;
    } else {
      return 1;
    }
  } else {
    return 0;
  }
}

bool PINCONFIG::testPrint() {
  digitalWrite(PIN_SWPROBE3, HIGH);
  delay(1); //For stability
  bool sw = digitalRead(PIN_SWCHECK1);
  digitalWrite(PIN_SWPROBE3, LOW);
  delay(1); //For stability
  return sw;
}

bool PINCONFIG::testAM() {
  digitalWrite(PIN_SWPROBE3, HIGH);
  delay(1); //For stability
  bool sw = digitalRead(PIN_SWCHECK2);
  digitalWrite(PIN_SWPROBE3, LOW);
  delay(1); //For stability
  return sw;
}

int PINCONFIG::testKeyRow() {
  //Creates a bitwise number for the keys pressed in the columns for that row
  int keys = digitalRead(PIN_KEYCHECK1) << 1;
  keys += digitalRead(PIN_KEYCHECK2); keys << 1;
  keys += digitalRead(PIN_KEYCHECK3); keys << 1;
  keys += digitalRead(PIN_KEYCHECK4); keys << 1;
  keys += digitalRead(PIN_KEYCHECK5); keys << 1;
  keys += digitalRead(PIN_KEYCHECK6); keys << 1;
  keys += digitalRead(PIN_KEYCHECK7); keys << 1;
  keys += digitalRead(PIN_KEYCHECK8);
  return keys;
}

bool PINCONFIG::testKeys(KeyRaw &keys) {
  digitalWrite(PIN_KEYPROBE1, HIGH);
  delay(1); //For stability
  keys.key1 = testKeyRow();
  digitalWrite(PIN_KEYPROBE1, LOW);
  digitalWrite(PIN_KEYPROBE2, HIGH);
  delay(1); //For stability
  keys.key2 = testKeyRow();
  digitalWrite(PIN_KEYPROBE2, LOW);
  digitalWrite(PIN_KEYPROBE3, HIGH);
  delay(1); //For stability
  keys.key3 = testKeyRow();
  digitalWrite(PIN_KEYPROBE3, LOW);
  digitalWrite(PIN_KEYPROBE4, HIGH);
  delay(1); //For stability
  keys.key4 = testKeyRow();
  digitalWrite(PIN_KEYPROBE4, LOW);
  return ((keys.key1|keys.key2|keys.key3|keys.key4)!=0);
}

//This function will delay execution UNLESS a key is pressed
bool PINCONFIG::testKeysDelay(int delayTime, KeyRaw &keys) {
  unsigned long startTime = millis();
  bool keypressed = false;
  while ((!keypressed) && (millis()-startTime <= delayTime)) {
    keypressed = testKeys(keys);
  }
  return keypressed;
}
