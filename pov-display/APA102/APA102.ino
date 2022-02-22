// Native APA102 implementation
// Using SPI
#include <SPI.h>

#define LEDS 20
#define BRIGHTNESS 5

SPIClass ledSPI(VSPI);

void setup() {
  ledSPI.begin();
}

void loop() {
  //Not sure if really necessary
  //SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  //Start frame: 32 bits of zeros
  ledSPI.transfer(0x00);
  ledSPI.transfer(0x00);
  ledSPI.transfer(0x00);
  ledSPI.transfer(0x00);
  for (uint8_t i = 0; i<LEDS; i++) {
    ledSPI.transfer(0xE0 + BRIGHTNESS);
    ledSPI.transfer(random(256));
    ledSPI.transfer(random(256));
    ledSPI.transfer(random(256));
  }
  //32 bits of zeros are enough for 64 LEDs, might re
  // End frame: 8+8*(leds >> 4) clock cycles
  for (uint8_t i = 0; i<LEDS; i+=16) {
    ledSPI.transfer(0x00); // 8 more clock cycles
  }
  //Not sure if really necessary
  //SPI.endTransaction();
  delay(500); // Half a second delay
}
