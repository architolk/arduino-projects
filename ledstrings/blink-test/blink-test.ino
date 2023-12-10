#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 150
#define MIN_BRIGHT 30
#define MAX_BRIGHT 100

// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI
#define DATA_PIN 6
#define CLOCK_PIN 13

// Define the array of leds
CRGB leds[NUM_LEDS];

//Globals
int brightness;
int delta;
int offset;
unsigned long stardelay;
unsigned long starttime;

void setup() {
  delay(1000); //One second delay to fire everything up 
  FastLED.addLeds<WS2813, DATA_PIN, GRB>(leds, NUM_LEDS);//.setCorrection(TypicalLEDStrip);
  InitLEDSYellowFlow();
  FastLED.show();
  brightness = 100;
  delta = -1;
  offset = 0;
  ResetStarDelay();
}

void ResetStarDelay() {
  stardelay = random(1000,10000);
  starttime = millis();
}

void InitLEDSYellowFlow() {
  for (int i=0; i<NUM_LEDS; i++) {
    setLEDColor(i,0,100);
  }
}

void setLEDColor(int i, int o, int b) {
  float rad = 0.15 * i;
  float wave = (0.5+0.5*sin(rad))*b/100;
  byte red = round(247*wave);
  byte green = round(60*wave);
  byte blue = round(4*wave);
  leds[(i+o)%NUM_LEDS].setRGB(8+red,2+green,blue);
}

void InitLEDSRedGreen() {
  int j=0;
  byte clr = 0;
  for (int i=0; i<NUM_LEDS; i++) {
    j++;
    if (j>10) {
      j=0;
      clr = 1-clr;
    }
    if (clr==0) {
      leds[i] = 0x400000;
    } else {
      leds[i] = 0x004000;
    }
  }
}

void loop() {
  delay(100);
  for (int i=0; i<NUM_LEDS; i++) {
    setLEDColor(i,offset,brightness);
  }
  FastLED.show();
  brightness=brightness+delta;
  if (brightness<MIN_BRIGHT) {
    brightness=MIN_BRIGHT;
    delta=1;
  }
  if (brightness>MAX_BRIGHT) {
    brightness=MAX_BRIGHT;
    delta=-1;
  }
  /*
  offset++;
  if (offset>NUM_LEDS) {
    offset=0;
  }
  */
  unsigned long currenttime = millis();
  if (currenttime - starttime > stardelay) {
    int lednr = random(NUM_LEDS);
    CRGB original = leds[lednr];
    leds[lednr] = 0xFFFFFF;
    FastLED.show();
    delay(100);
    leds[lednr] = original;
    FastLED.show();
    ResetStarDelay();
  }
}
