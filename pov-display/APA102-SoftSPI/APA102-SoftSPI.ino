// Native APA102 implementation
// Using SPI
// Beetle pins:
// D2 = MOSI (DI)
// D3 = SCK (CI)
// D5 = MISO (not used, just for reference)
#include <SPI.h>
//#include <SoftSPI.h>

#define LEDS 60
#define BRIGHTNESS 5

#define BUFLENGTH 180
byte buffer[BUFLENGTH];

uint8_t idx = 0;
bool doOnce = true;

//SPIClass ledSPI(VSPI);
//SoftSPI ledSPI(D2,D5,D3);
SPIClass ledSPI(HSPI);

void setup() {
  pinMode(D4,OUTPUT);
}

void loop() {
  if (doOnce) {
    doOnce=false;
    digitalWrite(D4,HIGH);
    //ledSPI.begin();
    ledSPI.begin(D3,D5,D2);
  }
  for (uint8_t i=0; i<BUFLENGTH; i++) {
    //buffer[i]=random(256);
    buffer[i]=0;
  }
  buffer[idx*3]=255;
  updateLEDs();
  delay(2); // 2 ms delay
  idx++;
  if (idx>=LEDS) {
    idx=0;
  }
}

void updateLEDs() {
  //Not sure if really necessary
  ledSPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  //Start frame: 32 bits of zeros
  ledSPI.write(0x00);
  ledSPI.write(0x00);
  ledSPI.write(0x00);
  ledSPI.write(0x00);
  for (uint8_t i = 0; i<LEDS; i++) {
    ledSPI.write(0xE0 + BRIGHTNESS);
    ledSPI.write(buffer[i*3]);
    ledSPI.write(buffer[i*3+1]);
    ledSPI.write(buffer[i*3+2]);
  }
  //32 bits of zeros are enough for 64 LEDs, might re
  // End frame: 8+8*(leds >> 4) clock cycles
  for (uint8_t i = 0; i<LEDS; i+=16) {
    ledSPI.write(0x00); // 8 more clock cycles
  }
  //Not sure if really necessary
  ledSPI.endTransaction();
}
