// Native APA102 implementation
// Using SPI
#include <SPI.h>

#define LEDS 20
#define BRIGHTNESS 5

#define BUFLENGTH 180
byte buffer[BUFLENGTH];

SPIClass ledSPI(VSPI);

void setup() {
  ledSPI.begin();
}

void loop() {
  for (uint8_t i=0; i<BUFLENGTH; i++) {
    buffer[i]=random(256);
  }
  updateLEDs();
  delay(500); // Half a second delay
}

void updateLEDs() {
  //Not sure if really necessary
  //SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  //Start frame: 32 bits of zeros
  ledSPI.transfer(0x00);
  ledSPI.transfer(0x00);
  ledSPI.transfer(0x00);
  ledSPI.transfer(0x00);
  for (uint8_t i = 0; i<LEDS; i++) {
    ledSPI.transfer(0xE0 + BRIGHTNESS);
    ledSPI.transfer(buffer[i]);
    ledSPI.transfer(buffer[i+1]);
    ledSPI.transfer(buffer[i+2]);
  }
  //32 bits of zeros are enough for 64 LEDs, might re
  // End frame: 8+8*(leds >> 4) clock cycles
  for (uint8_t i = 0; i<LEDS; i+=16) {
    ledSPI.transfer(0x00); // 8 more clock cycles
  }
  //Not sure if really necessary
  //SPI.endTransaction();
}
