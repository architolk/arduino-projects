// Example using serial monitoring to retrieve values from SD card

#include "SD.h"
#define SD_ChipSelectPin 10

void setup()
{
  Serial.begin(9600);
  if(!SD.begin(SD_ChipSelectPin))
  {
    Serial.println("SD fail");
    return;
  }

  File dataFile = SD.open("sample.wav");

  if (dataFile) {
    while (dataFile.available()) {
      //Reads one byte from the file, and send it to the serial port
      Serial.write(dataFile.read());
    }
    dataFile.close();
  }
  else {
    // if the file isn't open, pop up an error:
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
