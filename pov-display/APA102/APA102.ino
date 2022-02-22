// Native APA102 implementation
// Using SPI
#include <SPI.h>

#define LEDS 5
#define BRIGHTNESS 15

setup() {
  SPI.begin();
}

loop() {
  //Not sure if really necessary
  //SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  //Start frame: 32 bits of zeros
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  for (uint8_t i = 0; i<LEDS; i++) {
    SPI.transfer(0xE0 + BRIGHTNESS);
    SPI.transfer(random(256));
    SPI.transfer(random(256));
    SPI.transfer(random(256));
  }
  //32 bits of zeros are enough for 64 LEDs, might re
  // End frame: 8+8*(leds >> 4) clock cycles
  for (uint8_t i = 0; i<LEDS; i+=16) {
    SPI.transfer(0x00); // 8 more clock cycles
  }
  //Not sure if really necessary
  //SPI.endTransaction();
  delay(1000); // One second delay
}
