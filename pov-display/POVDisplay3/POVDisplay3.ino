// Native APA102 implementation
// Using SPI
#include <SPI.h>

// BLE Stack ESP32
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//Using the most commonly used UART service for BLE
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

//HALLPIN will be D7
//#define HALLPIN 16
//READYPIN wil be D4
//#define READYPIN 17
//#define LEDPIN 22

#define LEDS 60
#define BRIGHTNESS 3

#define BUFLENGTH 180
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

volatile unsigned long rotationTime, timeOld, timeNew;
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
          for (int i = 0; i < rxValue.length(); i++) {
            //Failsafe: length should be BUFLENGTH, but just in case!
            if (i<BUFLENGTH) {
              buffer[theta][i]=rxValue[i]
            }
          }
          // Failsafe: if length < BUFLENGTH, pad with zeros
          for (int i = rxValue.length(); i < BUFLENGTH; i++) {
            buffer[theta]i]=0;
          }
          // Request next line, or finish when all lines are received
          theta++;
          if (theta<RESOLUTION) {
            requestThetaIndex();
          } else {
            imgLoaded = true;
            imgLoading = false;
            updateLEDs();
          }
        }
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
  pinMode(D4, OUTPUT);
  //pinMode(LEDPIN, OUTPUT);
  pinMode(D7, INPUT_PULLUP);

  //set rotation time to 1 second (120 lines per second)
  rotationTime = 1000000;

  for (uint8_t i=0; i<BUFLENGTH; i++) {
    buffer[0][i]=0;
  }
  buffer[0][0] = 255;
  buffer[0][4] = 255;
  buffer[0][8] = 255;

  setupBLE();

  timeNew = micros();
  timeOld = timeNew;
  attachInterrupt(digitalPinToInterrupt(D7), magnetPresent, FALLING);
}

IRAM_ATTR void magnetPresent() {
  timeNew = micros();
  rotationTime = timeNew - timeOld;
  timeOld = timeNew;
}

void loop() {
  if (doOnce==true) {
    //digitalWrite(LEDPIN, HIGH);
    //Only update the LEDs to random the first time, next will be the serial bluetooth
    digitalWrite(D4, HIGH);
    ledSPI.begin(D3,D5,D2);
    updateLEDs();
    doOnce=false;
  }
  if (imgLoaded==true) {
    //Time do to some rotation stuff!
    spinNew = micros();
    if ((spinNew-spinOld)*RESOLUTION>currentRotationTime) {
      spinOld = spinNew;
      rotor++;
      if (rotor==RESOLUTION) {
        rotor=0;
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
