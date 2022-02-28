// Native APA102 implementation
// Using SPI
#include <SPI.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

#define LEDS 20
#define BRIGHTNESS 5

#define BUFLENGTH 180
#define RESOLUTION 120
byte buffer[RESOLUTION][BUFLENGTH];

SPIClass ledSPI(VSPI);

int startupCount = -3;
byte theta = 0;
byte rotor = 0;
boolean doOnce = true;
boolean imgLoaded = false;

void setup() {
  for (uint8_t i=0; i<BUFLENGTH; i++) {
    buffer[0][i]=0;
  }
  buffer[0][0] = 255;
  buffer[0][4] = 255;
  buffer[0][8] = 255;
  SerialBT.begin("ESP32test"); //Bluetooth device name
  ledSPI.begin();
}

void loop() {
  if (doOnce==true) {
    //Only update the LEDs to random the first time, next will be the serial bluetooth
    updateLEDs();
    doOnce=false;
  }
  if (SerialBT.available()) {
    byte received = SerialBT.read();
    if (startupCount==-3) {
      if (received=255) {
        startupCount++;
      } else {
        theta=0;
      }
    } else if (startupCount==-2) {
      if (received==0) {
        startupCount++;
      } else {
        startupCount=-3;
        theta=0;
      }
    } else if (startupCount==-1) {
      if (received==128) {
        startupCount++;
        theta=0;
        imgLoaded=false; //Image in processing of loading, do not show during loading of new image
        requestThetaIndex(0);
      } else {
        startupCount=-3;
        theta=0;
      }
    } else {
      //Start receiving
      buffer[theta][startupCount]=received;
      startupCount++;
      if (startupCount==BUFLENGTH) {
        startupCount=0;
        theta++;
        if (theta<RESOLUTION) {
          //Resolution not reached, request next line
          requestThetaIndex(theta);
        } else {
          startupCount=-3;
          theta=0;
          imgLoaded=true;
          updateLEDs();
        }
      }
    }
  } else {
    if (imgLoaded==true) {
      //Time do to some rotation stuff!
      delay(100); //Small delay: should change to the real framerate of the rotor
      rotor++;
      if (rotor==RESOLUTION) {
        rotor=0;
      }
      updateLEDs();
    }
  }
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
    //Buffer contains colors as RGB, the APA102 expects BGR
    ledSPI.transfer(buffer[rotor][i*3+2]); //Blue
    ledSPI.transfer(buffer[rotor][i*3+1]); //Green
    ledSPI.transfer(buffer[rotor][i*3]);   //Red
  }
  //32 bits of zeros are enough for 64 LEDs, might re
  // End frame: 8+8*(leds >> 4) clock cycles
  for (uint8_t i = 0; i<LEDS; i+=16) {
    ledSPI.transfer(0x00); // 8 more clock cycles
  }
  //Not sure if really necessary
  //SPI.endTransaction();
}

void requestThetaIndex(byte theta) {
  //Failsafe: theta should not be langer than RESOLUTION
  if (theta<RESOLUTION) {
    SerialBT.write(theta);
    delay(20);
  }
}
