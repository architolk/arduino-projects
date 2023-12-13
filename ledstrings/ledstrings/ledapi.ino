/*
 * LED API
 *
 * The code for controling the LEDs
 *
 */

#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 150
#define MIN_BRIGHT 30
#define MAX_BRIGHT 100

#define DATA_PIN 6

//Globals LED-API (refactor - should be API globals)
int brightness;
int delta;
int offset;

CRGB original;
int blinkingLedNr;
boolean blinkActive;

time_t starDelay;
time_t starBlinkTime;
time_t waveTime;

// Define the array of leds
CRGB leds[NUM_LEDS];

void setupLEDs() {
  FastLED.addLeds<WS2813, DATA_PIN, GRB>(leds, NUM_LEDS);//.setCorrection(TypicalLEDStrip);
  initLEDSYellowFlow();
  FastLED.show();
  brightness = 100;
  delta = -1;
  offset = 0;
  resetStarDelay(false);
  resetWaveTimer();
}

void resetStarDelay(boolean _blinkActive) {
  blinkActive = _blinkActive;
  if (blinkActive) {
    starDelay = 100;
  } else {
    starDelay = random(1000,10000);
  }
  starBlinkTime = millis();
}

void resetWaveTimer() {
  waveTime = millis();
}

void initLEDSYellowFlow() {
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

void initLEDSRedGreen() {
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

void checkLedString() {

  currentTime = millis();
  if ((currentTime - waveTime) > 100) {
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
    resetWaveTimer();
  }

 if ((currentTime - starBlinkTime) > starDelay) {
   // Reset to original when blink is active, otherwise: set a LED to bright white
   // Blink will be active for 0.1s
   if (blinkActive) {
     leds[blinkingLedNr] = original;
   } else {
     blinkingLedNr = random(NUM_LEDS);
     original = leds[blinkingLedNr];
     leds[blinkingLedNr] = 0xFFFFFF;
   }
   FastLED.show();
   resetStarDelay(!blinkActive);
 }
}
