// Native APA102 implementation
// Using SPI
#include <SPI.h>

// BLE Stack ESP32
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//SPI File system
#include <FS.h>
#include <SPIFFS.h>

//Set to false after first time
#define FORMAT_SPIFFS_IF_FAILED false

//Using the most commonly used UART service for BLE
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

//MOSIPIN will be D3
#define MOSIPIN 26
//CLKPIN will be D4
#define CLKPIN 27
//HALLPIN will be D2
#define HALLPIN 25
//READYPIN wil be D7
#define READYPIN 13

#define LEDS 83
#define BRIGHTNESS 3

//BUFLENGTH shoulde be at least 3x LEDS count
#define BUFLENGTH 249
#define RESOLUTION 120
byte buffer[RESOLUTION][BUFLENGTH];

SPIClass ledSPI(HSPI);

// BLE global variables
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

uint8_t theta = 0;
byte rotor = 0;
boolean doOnce = true;
boolean imgLoaded = false;
boolean imgLoading = false;

boolean filesReady = false;

volatile unsigned long rotationTime, timeOld, timeNew;
volatile boolean magnetHit = false;
unsigned long currentRotationTime,spinOld,spinNew = 0;

// ===========
// BLE stuff
// ===========

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        if ((!imgLoading) && (rxValue[0]==255) && (rxValue[1]==0) && (rxValue[2]==128)) {
          imgLoaded = false;
          imgLoading = true;
          theta = 0;
          requestThetaIndex();
        } else {
          if (imgLoading) {
            for (int i = 0; i < rxValue.length(); i++) {
              //Failsafe: length should be BUFLENGTH, but just in case!
              if (i<BUFLENGTH) {
                buffer[theta][i]=rxValue[i];
              }
            }
            // Failsafe: if length < BUFLENGTH, pad with zeros
            for (int i = rxValue.length(); i < BUFLENGTH; i++) {
              buffer[theta][i]=0;
            }
            // Request next line, or finish when all lines are received
            theta++;
            if (theta<RESOLUTION) {
              requestThetaIndex();
            } else {
              imgLoaded = true;
              imgLoading = false;
            }
          } else {
            switch (rxValue[0]) {
              case 115:
                //Operation "s": Save file to flash
                if (filesReady && (rxValue.length()>1)) {
                  rxValue[0] = 47; //Forward slash, so making rxValue a real filename
                  File file = SPIFFS.open(rxValue.c_str(),FILE_WRITE);
                  if (file) {
                    for (int i = 0; i < RESOLUTION; i++) {
                      for (int j = 0; j < BUFLENGTH; j++) {
                        file.write(buffer[i][j]);
                      }
                    }
                    file.close();
                    sendText("saved");
                  }
                }
                break;
              case 114:
                //Operation "r": Read file from flash
                if (filesReady && (rxValue.length()>1)) {
                  rxValue[0] = 47; //Forward slash, so making rxValue a real filename
                  File file = SPIFFS.open(rxValue.c_str());
                  if (file) {
                    for (int i = 0; i < RESOLUTION; i++) {
                      for (int j = 0; j < BUFLENGTH; j++) {
                        if (file.available()) {
                          buffer[i][j] = file.read();
                        } else {
                          //Failsafe: should not occur, but if file is to small - add zeros
                          buffer[i][j]= 0;
                        }
                      }
                    }
                    file.close();
                    sendText("loaded");
                  }
                }
                break;
            }
          }
        }
      }
    }

    void sendText(std::string text) {
      if (deviceConnected) {
        pTxCharacteristic->setValue(text);
        pTxCharacteristic->notify();
        delay(10);
      }
    }

    void requestThetaIndex() {
      if (deviceConnected) {
        if (theta<RESOLUTION) {
          pTxCharacteristic->setValue(&theta, 1);
          pTxCharacteristic->notify();
          delay(10); // bluetooth stack will go into congestion, if too many packets are sent
        }
      }
    }

};

void setupBLE() {
  // Create the BLE Device
  BLEDevice::init("POV Display");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
}

// =========
// End BLE stuff
// =========

void setup() {
  pinMode(READYPIN, OUTPUT);
  //pinMode(LEDPIN, OUTPUT);
  pinMode(HALLPIN, INPUT_PULLUP);

  //set rotation time to 0.28 second (429 lines per second)
  rotationTime = 280000;

  for (uint8_t i=0; i<BUFLENGTH; i++) {
    buffer[0][i]=0;
  }
  buffer[0][0] = 255;
  buffer[0][4] = 255;
  buffer[0][8] = 255;

  setupBLE();

  //Initialize SPIFFS
  filesReady = SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED);

  timeNew = micros();
  timeOld = timeNew;
  //Hall sensor doesn't work - skip for now
  attachInterrupt(digitalPinToInterrupt(HALLPIN), magnetPresent, FALLING);
}

IRAM_ATTR void magnetPresent() {
  timeNew = micros();
  rotationTime = timeNew - timeOld;
  timeOld = timeNew;
  magnetHit = true; //Flag that the magnet is hit, so rotor should be 60 (bottom of picture)
}

void loop() {
  if (doOnce==true) {
    //digitalWrite(LEDPIN, HIGH);
    //Only update the LEDs to random the first time, next will be the serial bluetooth
    digitalWrite(READYPIN, HIGH);
    ledSPI.begin(CLKPIN,D5,MOSIPIN);
    updateLEDs();
    doOnce=false;
  }
  if (imgLoaded==true) {
    //Time do to some rotation stuff!
    spinNew = micros();
    if ((spinNew-spinOld)*RESOLUTION>currentRotationTime) {
      spinOld = spinNew;
      if (magnetHit) {
        magnetHit = false;
        rotor = 30;
      } else {
        rotor++;
        if (rotor==RESOLUTION) {
          rotor=0;
        }
      }
      updateLEDs();
    }
    if (currentRotationTime!=rotationTime) {
      currentRotationTime = rotationTime;
    }
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
  // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
  }
}

void updateLEDs() {
  ledSPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  //Start frame: 32 bits of zeros
  ledSPI.write(0x00);
  ledSPI.write(0x00);
  ledSPI.write(0x00);
  ledSPI.write(0x00);
  for (uint8_t i = 0; i<LEDS; i++) {
    ledSPI.write(0xE0 + BRIGHTNESS);
    //Buffer contains colors as RGB, the APA102 expects BGR
    ledSPI.write(buffer[rotor][i*3+2]); //Blue
    ledSPI.write(buffer[rotor][i*3+1]); //Green
    ledSPI.write(buffer[rotor][i*3]);   //Red
  }
  //32 bits of zeros are enough for 64 LEDs, might re
  // End frame: 8+8*(leds >> 4) clock cycles
  for (uint8_t i = 0; i<LEDS; i+=16) {
    ledSPI.write(0x00); // 8 more clock cycles
  }
  ledSPI.endTransaction();
}
