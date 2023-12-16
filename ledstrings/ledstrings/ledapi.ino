/*
 * LED API
 *
 * The code for controling the LEDs
 *
 */

#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 150
#define NUM_STRIPS 3
#define MIN_BRIGHT 30

//With one strip, between 1s-10s seems right, with three strips, interval should be a 1/3 of these values
//Max blink interval is in miliseconds
#define MIN_BLINK_INTERVAL 333
#define MAX_BLINK_INTERVAL 3333

//PINS per Strip. Mind that this must match NUM_STRIPS and setupLEDs() should reflect this!
#define DATA_PIN0 5
#define DATA_PIN1 6
#define DATA_PIN2 7

const int CHASER_LIGHTS[] PROGMEM = {0,10,40,128,255,255,128,40,10,0};

//Globals LED-API (refactor - should be API globals)
int brightness;
int delta;
int offset;

CRGB original;
int blinkingLedNr;
int blinkingStripNr;
boolean blinkActive;

time_t starDelay;
time_t starBlinkTime;
time_t waveTime;

// Define the array of leds
// Every strip has a seperate array
CRGB leds[NUM_STRIPS][NUM_LEDS];

void setupLEDs() {
  //Initialize all strips - this cannot be done with a for loop, because the pin value must be a constant...
  FastLED.addLeds<WS2813, DATA_PIN0, GRB>(leds[0], NUM_LEDS);
  FastLED.addLeds<WS2813, DATA_PIN1, GRB>(leds[1], NUM_LEDS);
  FastLED.addLeds<WS2813, DATA_PIN2, GRB>(leds[2], NUM_LEDS);
  resetStarDelay(false);
  resetWaveTimer();
  initLEDs();
}

void initLEDs() {
  switch (ledStatus) {
    case LEDS_ON:
      switch (ledMode) {
        case MODE_DISCRETE:
          initLEDSYellowFlow();
          brightness = 100;
          delta = -1;
          offset = 0;
          break;
        case MODE_CHRISTMAS:
          initLEDSRedGreen();
          break;
        case MODE_NL:
          initLEDSRedWhiteBlue();
          break;
        case MODE_CHASER:
          initLEDsOff(); //Chaser starts with all LEDs off
          delta = 1;
          offset = 0;
          break;
        case MODE_RAINBOW:
          initRainbow();
          break;
        case MODE_STAIRCASE:
          initLEDsOff(); //Staircase starts with all LEDs off
          offset = 0;
          break;
        default:
          initLEDSYellowFlow();
          break;
      }
      break;
    case LEDS_OFF:
      initLEDsOff();
      break;
    case LEDS_DIMMED:
    default:
      initLEDsOff(); //Dimmed LEDs starts with all LEDs off
      break;
  }
  FastLED.show();
}

void updateLEDs() {
  switch (ledStatus) {
    case LEDS_ON:
      switch (ledMode) {
        case MODE_DISCRETE:
          checkLedString();
          break;
        case MODE_STAIRCASE:
          checkStaircaseString();
          break;
        case MODE_CHASER:
          checkChaserString();
          break;
        case MODE_NL:
        case MODE_RAINBOW:
        case MODE_CHRISTMAS:
        default:
          break; //No movement in these modes
      }
      break;
    case LEDS_DIMMED:
      checkDimmedString();
      break;
    case LEDS_OFF:
    default:
      break; //Do nothing when LEDs are off
  }
}

void resetStarDelay(boolean _blinkActive) {
  blinkActive = _blinkActive;
  if (blinkActive) {
    starDelay = 100;
  } else {
    starDelay = random(MIN_BLINK_INTERVAL,MAX_BLINK_INTERVAL);
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
  for (int x=0; x<NUM_STRIPS; x++) {
    leds[x][(i+o)%NUM_LEDS].setRGB(8+red,2+green,blue);
  }
}

void initLEDsOff() {
  for (int x=0; x<NUM_STRIPS; x++) {
    for (int i=0; i<NUM_LEDS; i++) {
      leds[x][i] = 0;
    }
  }
}

void initLEDSRedGreen() {
  for (int x=0; x<NUM_STRIPS; x++) {
    int j=0;
    byte clr = 0;
    for (int i=0; i<NUM_LEDS; i++) {
      j++;
      if (j>10) {
        j=0;
        clr = 1-clr;
      }
      if (clr==0) {
        leds[x][i].setRGB(maxBrightness,0,0);
      } else {
        leds[x][i].setRGB(0,maxBrightness,0);
      }
    }
  }
}

void initLEDSRedWhiteBlue() {
  for (int x=0; x<NUM_STRIPS; x++) {
    for (int i=0; i<NUM_LEDS; i++) {
      if (i<(NUM_LEDS/3)) {
        leds[x][i].setRGB(maxBrightness,0,0);
      } else if (i<(NUM_LEDS*2/3)) {
        leds[x][i].setRGB(maxBrightness,maxBrightness,maxBrightness);
      } else {
        leds[x][i].setRGB(0,0,maxBrightness);
      }
    }
  }
}

void initRainbow() {
  for (int x=0; x<NUM_STRIPS; x++) {
    for (int i=0; i<NUM_LEDS; i++) {
      leds[x][i].setHSV(i*255/(NUM_LEDS-1),255,maxBrightness);
    }
  }
}

void checkDimmedString() {
  //Only one strip (the first one) will be used for sending a red blinking light
  currentTime = millis();
  if ((currentTime - waveTime) > 1000) {
    for (int i=0; (i<3) && (i<NUM_LEDS); i++) {
      leds[0][NUM_LEDS - i] = ((leds[0][NUM_LEDS - i]==0) ? 0x400000 : 0);
    }
    FastLED.show();
    resetWaveTimer();
  }
}

void checkChaserString() {
  currentTime = millis();
  if ((currentTime - waveTime) > 10) {
    for (int x=0; x<NUM_STRIPS; x++) {
      for (int i=0; (i<10) && (i<NUM_LEDS); i++) {
        leds[x][i+offset].setRGB(CHASER_LIGHTS[i],0,0);
      }
    }
    offset = offset + delta;
    if (offset==0) {
      delta = 1;
    }
    if (offset>(NUM_LEDS-10)) {
      offset = offset = (NUM_LEDS<10) ? 0 : NUM_LEDS-10;
      delta = -1;
    }
    FastLED.show();
    resetWaveTimer();
  }
}

void setStaircaseStep(int o, int b) {
  for (int x=0; x<NUM_STRIPS; x++) {
    for (int i=1; i<11; i++) {
      if ((o-i)>0) {
        leds[x][o-i].setRGB(b,b,b);
      }
    }
  }
}

void checkStaircaseString() {
  if (offset<=NUM_LEDS) {
    currentTime = millis();
    if ((currentTime - waveTime) > 1000) {
      if (offset>0) {
        setStaircaseStep(NUM_LEDS-offset+10,10);
      }
      setStaircaseStep(NUM_LEDS-offset,maxBrightness);
      offset = offset + 10;
      FastLED.show();
      resetWaveTimer();
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
    if (brightness>maxBrightness) {
     brightness=maxBrightness;
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
     leds[blinkingStripNr][blinkingLedNr] = original;
   } else {
     blinkingLedNr = random(NUM_LEDS);
     blinkingStripNr = random(NUM_STRIPS);
     original = leds[blinkingStripNr][blinkingLedNr];
     leds[blinkingStripNr][blinkingLedNr] = 0xFFFFFF;
   }
   FastLED.show();
   resetStarDelay(!blinkActive);
 }
}
