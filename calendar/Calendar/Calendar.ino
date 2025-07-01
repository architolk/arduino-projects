#include "src/e-Paper/DEV_Tools.h"
#include "src/e-Paper/EInkDisplay.h"
#include "src/Graphics/EPDCanvas.h"
#include "Config.h"

#include "src/Fonts/FreeSansBold8pt7b.h"
#include "src/Fonts/FreeSansBold16pt7b.h"
#include "src/Fonts/FreeSans10pt7b.h"
#include "src/Fonts/FreeSans12pt7b.h"
#include "src/Fonts/FreeSans16pt7b.h"
#include "src/Fonts/FreeSans18pt7b.h"
#include "src/Fonts/FreeSans20pt7b.h"
#include "src/Fonts/FreeSans30pt7b.h"
#include "src/Fonts/WeatherIcons26pt7b.h"
#include "src/Fonts/WeatherIcons36pt7b.h"
#include "src/Fonts/WeatherIcons50pt7b.h"

//Uncomment this to keep the display state persistant (the display won't be cleared, leaving the last image behind - not recommended if e-Paper is stored)
//#define KEEP_DISPLAY_STATE

//Displays
EInkDisplay TopDisplay(TOP_PIN_CS, TOP_PIN_DC, TOP_PIN_RST, TOP_PIN_BUSY, TOP_PIN_PWR);;
EInkDisplay BottomDisplay(BOTTOM_PIN_CS, BOTTOM_PIN_DC, BOTTOM_PIN_RST, BOTTOM_PIN_BUSY, BOTTOM_PIN_PWR);;

//Canvas
EPDCanvas canvas(EPD_7IN5B_V2_WIDTH, EPD_7IN5B_V2_HEIGHT);

void initialize() {
  pinMode(LED_PIN_ESP32, OUTPUT); //Set LED pin to output
  digitalWrite(LED_PIN_ESP32, LOW); // Set LED pin low, indicating that we have started!

  //Setup pins according to config
  TopDisplay.setupPins();
  //BottomDisplay.setupPins();

	// spi
	SPI.begin(); //Should probable to SPI.begin(SPI_PIN_SCK, SPI_PIN_MISO, SPI_PIN_MOSI)
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0)); //SPI pins are the default ones...

  Serial.print("POC ePaper test\r\n");
}

void updateTopDisplay() {

  Serial.print("TopDisplay: e-Paper Init and Clear...\r\n");
  TopDisplay.activate(); //This will power on the display, and set the chip-select to this particular display

  //Initialize screen
  TopDisplay.init();
  //1 = White, 0 = Black
  canvas.fillScreen(1);      // fill background
  canvas.setTextColor(0, 1); // black text, white background

  //Just a small piece of the action
  canvas.setFont(&FreeSans30pt7b);
  canvas.drawText(4,55,"22");
  canvas.setFont(&FreeSans16pt7b);
  canvas.drawText(80,38,"November");
  canvas.setFont(&FreeSans18pt7b);
  canvas.drawText(4,100,"Woensdag");

  // done drawing, so send it off to the display
  TopDisplay.writeCanvas(&canvas, EPD_BLACK_WHITE_LAYER);

  //A bit confusing, but now: 0 = White, 1 = Red
  canvas.fillScreen(0);  // fill backgrund;

  // done drawing, so send it off to the display
  // NB: You should always end with "1" even if no red layer is present (because only at "1" the display is turned on!)
  TopDisplay.writeCanvas(&canvas, EPD_WHITE_RED_LAYER);  // 1 = red layer layer

  Serial.print("EPD_Display\r\n");

  #ifndef KEEP_DISPLAY_STATE

    Serial.print("Wait 5 seconds before display is cleared\r\n");
    DEV_Delay_ms(5000);

    Serial.print("Clear...(to put the e-Paper in it's original clear screen)\r\n");
    TopDisplay.init();
    TopDisplay.clear();
  #endif

  Serial.print("Goto Sleep...\r\n");
  TopDisplay.sleep();

  // close PWR
  Serial.print("close PWR, Module enters 0 power consumption ...\r\n");
  TopDisplay.deactivate();

}

void setup() {

  //Serial - for debugging only
  Serial.begin(115200);

  if (!canvas.getBuffer()) {
    Serial.print("ERROR: Could not allocate buffer for GFXcanvas1\r\n");
  } else {
    initialize();
    updateTopDisplay();
  }

  digitalWrite(LED_PIN_ESP32, HIGH); // Set LED to high, indicating that we have finished the programm

}

void loop() {

}
