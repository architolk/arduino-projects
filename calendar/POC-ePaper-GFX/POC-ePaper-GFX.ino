#include "src/Config/DEV_Config.h"

#include "src/e-Paper/EPD_7in5b_V2.h"

#include <Adafruit_GFX.h>
#include <gfxfont.h>

//Uncomment this to keep the display state persistant (the display won't be cleared, leaving the last image behind - not recommended if e-Paper is stored)
//#define KEEP_DISPLAY_STATE

void setup() {

  DEV_Module_Init(); //Initialize PINs, SPI bus, Serial
  Serial.print("POC ePaper test\r\n");

  Serial.print("e-Paper Init and Clear...\r\n");
  EPD_7IN5B_V2_Init(); //Initialize the display
  EPD_7IN5B_V2_Clear(); //Turn on the display, clear
  DEV_Delay_ms(500);

  //Create a new GFX canvas (used as image cache)
  //Memory is allocated by GFX, not sure how to detect any errors
  GFXcanvas1 canvas(EPD_7IN5B_V2_WIDTH, EPD_7IN5B_V2_HEIGHT);
  GFXcanvas1 *_can = &canvas;   // create a pointer to the canvas object // because we are short of storrage
  if (!canvas.getBuffer()) {
    Serial.print("ERROR: Could not allocate buffer for GFXcanvas1\r\n");
  }
  else {

    //Initialize screen
    EPD_7IN5B_V2_Init();

    //1 = White, 0 = Black
    canvas.fillScreen(1);                                        // fill background

    for (int e = 0; e < 34; e++) {
      canvas.drawLine(e * 10, 0, 0, canvas.height() - e * 10, 0);
    }

    canvas.setTextColor(0, 1); // black text, white background
    canvas.setTextSize(2);               // 2X size text
    canvas.setCursor(200,200); // Position
    canvas.print("Hello World");  // Print a message

    // done drawing, so send it off to the display
    EPD_7IN5B_V2_WriteCanvas(_can, 0);

    //A bit confusing, but now: 0 = White, 1 = Red
    canvas.fillScreen(0);  // fill backgrund;

    canvas.setTextColor(1, 0); // Red text, white background
    canvas.setTextSize(2);               // 2X size text
    canvas.setCursor(200,300); // Position
    canvas.print("Hello World");  // Print a message

    // done drawing, so send it off to the display
    // NB: You should always end with "1" even if no red layer is present (because only at "1" the display is turned on!)
    EPD_7IN5B_V2_WriteCanvas(_can, 1);  // 1 = red layer layer

    Serial.print("EPD_Display\r\n");

#ifndef KEEP_DISPLAY_STATE

    Serial.print("Wait 5 seconds before display is cleared\r\n");
    DEV_Delay_ms(5000);

    Serial.print("Clear...(to put the e-Paper in it's original clear screen)\r\n");
    EPD_7IN5B_V2_Init();
    EPD_7IN5B_V2_Clear();
#endif

  }

  Serial.print("Goto Sleep...\r\n");
  EPD_7IN5B_V2_Sleep();

  // close PWR
  Serial.print("close PWR, Module enters 0 power consumption ...\r\n");
  DEV_Module_Exit();

}

void loop() {
  //Nothing - just a POC so one time execution
}
