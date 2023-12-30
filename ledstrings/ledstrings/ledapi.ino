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

//Necessary for the Fire animation
static byte heat[NUM_LEDS];
// CONFIG_COOLING = (55 * 10 / NUM_LEDS)+2 =
#define CONFIG_COOLING 6
#define CONFIG_SPARKING 120

//Necessary for the Meteor animation
#define CONFIG_METEORSIZE 10
#define CONFIG_TRAILDECAY 64

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
        case MODE_FIRE:
          initLEDsOff();
          break;
        case MODE_FIREWORKS:
        case MODE_METEOR:
          initLEDsOff();
          offset = 0;
          break;
        case MODE_SPARKLES:
          initLEDsOff();
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
        case MODE_CHRISTMAS:
          checkStarBlink(true);
          break;
        case MODE_FIRE:
          checkFireString();
          break;
        case MODE_METEOR:
          checkMeteorRainString(NUM_STRIPS); //All strips meteor
          break;
        case MODE_SPARKLES:
          checkSparklesString(0); //All strips sparkles
          break;
        case MODE_FIREWORKS:
          //Fireworks is a combination of meteor and sparkles for different strips
          checkMeteorRainString(1); //Only strip 0 meteor
          checkSparklesString(1); //All except strip 0 sparkles
          break;
        case MODE_NL:
        case MODE_RAINBOW:
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
  starBlinkTime = currentTime;
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
  if ((currentTime - waveTime) > 1000) {
    for (int i=1; (i<=3) && (i<NUM_LEDS); i++) {
      leds[0][NUM_LEDS - i] = ((leds[0][NUM_LEDS - i]==0) ? 0x400000 : 0);
    }
    FastLED.show();
    waveTime = currentTime;
  }
}

void checkChaserString() {
  if ((currentTime - waveTime) > 1) { //Very fast, testing...
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
    waveTime = currentTime;
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
    if ((currentTime - waveTime) > 1000) {
      if (offset>0) {
        setStaircaseStep(NUM_LEDS-offset+10,10);
      }
      setStaircaseStep(NUM_LEDS-offset,maxBrightness);
      offset = offset + 10;
      FastLED.show();
      waveTime = currentTime;
    }
  }
}

void checkLedString() {

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
    waveTime = currentTime;

    checkStarBlink(false);

    FastLED.show();
  }
}

void checkStarBlink(boolean doShow) {

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
    resetStarDelay(!blinkActive);
    if (doShow) {
      FastLED.show();
    }
  }

}

void setPixelHeatColor (int pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);

  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
  byte heatrampGreen = heatramp >> 1; // scale down to 0..128 for green

  // figure out which third of the spectrum we're in:
  for (int x=0; x<NUM_STRIPS; x++) {
    if( t192 > 0x80) {                     // hottest
      leds[x][pixel].setRGB(255,128,heatramp); //Compensate green
    } else if( t192 > 0x40 ) {             // middle
      leds[x][pixel].setRGB(255,heatrampGreen,0); //Compensate green
    } else {                               // coolest
      leds[x][pixel].setRGB(heatramp,0,0);
    }
  }
}

void checkFireString() {

  if ((currentTime - waveTime) > 8) {
    int cooldown;

    // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      cooldown = random(0, CONFIG_COOLING);

      if(cooldown>heat[i]) {
        heat[i]=0;
      } else {
        heat[i]=heat[i]-cooldown;
      }
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' near the bottom
    if( random(255) < CONFIG_SPARKING ) {
      int y = random(7);
      heat[y] = heat[y] + random(160,255);
      //heat[y] = random(160,255);
    }

    // Step 4.  Convert heat to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      setPixelHeatColor(j, heat[j] );
    }
    FastLED.show();
    waveTime = currentTime;
  }
}

void checkMeteorRainString(int strips) {

  if ((currentTime - waveTime) > 10) {

    for (int x=0; x<strips; x++) {
      // fade brightness all LEDs one step
      for(int i=0; i<NUM_LEDS; i++) {
        if (random(10)>5) { //Random trail decay
          leds[x][i].fadeToBlackBy(CONFIG_TRAILDECAY);
        }
      }

      // draw meteor
      for(int i = 0; i < CONFIG_METEORSIZE; i++) {
        if( ( offset-i <NUM_LEDS) && (offset-i>=0) ) {
          if ((offset + CONFIG_METEORSIZE) < NUM_LEDS) {
            leds[x][offset-i].setRGB(64, 10, 0); //Set different RGB for different meteor color
          } else {
            leds[x][offset-i].setRGB(255, 255, 255); //At the top - brightest setting
          }
        }
      }
    }

    offset++;
    if (offset>=NUM_LEDS+NUM_LEDS) {
      offset=0;
    }

    //A bit of a hack: only show when strips>1, the sparkles will be next!
    //Better would be to return false/true and do these settings in the main loop
    if (strips>1) {
      FastLED.show();
      waveTime = currentTime;
    }
  }

}

void checkSparklesString(int strips) {
  if ((currentTime - waveTime) > 10) {

    for (int x=strips; x<NUM_STRIPS; x++) {
      // fade brightness all LEDs one step
      for(int i=0; i<NUM_LEDS; i++) {
        if (random(10)>5) { //Random trail decay
          leds[x][i].fadeToBlackBy(CONFIG_TRAILDECAY); //Reuse from meteor rain
        }
      }

      // Draw sparkles
      for (int i=0; i < 4; i++) { //4 new sparkles...
        int j = random(NUM_LEDS);
        leds[x][j].setRGB(255,255,255);
      }

    }

    FastLED.show();
    waveTime = currentTime;
  }
}
