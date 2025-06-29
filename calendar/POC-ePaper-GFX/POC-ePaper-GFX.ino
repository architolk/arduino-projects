#include "src/Config/DEV_Config.h"

#include "src/e-Paper/EPD_7in5b_V2.h"

#include <EPDCanvas.h>
#include "src/Fonts/FreeSansBold8pt7b.h"
#include "src/Fonts/FreeSansBold16pt7b.h"
#include "src/Fonts/FreeSans12pt7b.h"
#include "src/Fonts/FreeSans16pt7b.h"
#include "src/Fonts/FreeSans20pt7b.h"
#include "src/Fonts/WeatherIcons26pt7b.h"
#include "src/Fonts/WeatherIcons36pt7b.h"

//Uncomment this to keep the display state persistant (the display won't be cleared, leaving the last image behind - not recommended if e-Paper is stored)
//#define KEEP_DISPLAY_STATE

void setup() {
  pinMode(2, OUTPUT); //Set LED pin to output
  digitalWrite(2, LOW); // Set LED pin low, indicating that we have started!

  DEV_Module_Init(); //Initialize PINs, SPI bus, Serial
  Serial.print("POC ePaper test\r\n");

  Serial.print("e-Paper Init and Clear...\r\n");
  EPD_7IN5B_V2_Init(); //Initialize the display
  EPD_7IN5B_V2_Clear(); //Turn on the display, clear
  DEV_Delay_ms(500);

  //Create a new GFX canvas (used as image cache)
  //Memory is allocated by GFX, not sure how to detect any errors
  EPDCanvas canvas(EPD_7IN5B_V2_WIDTH, EPD_7IN5B_V2_HEIGHT);
  EPDCanvas *_can = &canvas;   // create a pointer to the canvas object // because we are short of storrage
  if (!canvas.getBuffer()) {
    Serial.print("ERROR: Could not allocate buffer for GFXcanvas1\r\n");
  }
  else {

    //Initialize screen
    EPD_7IN5B_V2_Init();

    //1 = White, 0 = Black
    canvas.fillScreen(1);                                        // fill background

    canvas.setTextColor(0, 1); // black text, white background

    canvas.setFont(&FreeSansBold16pt7b);
    canvas.drawText(40,25,"3:00");
    canvas.drawText(200,25,"7:00");
    canvas.drawText(350,25,"12:00");
    canvas.drawText(510,25,"18:00");
    canvas.drawText(675,25,"23:00");

    for (int i = 0; i <5; i++) {
      //Weather icon
      canvas.setFont(&WeatherIcons36pt7b);
      canvas.drawText(10+160*i,90,"B");
      //Temperature
      canvas.setFont(&FreeSans16pt7b);
      canvas.drawText(80+160*i,65,"24");
      canvas.setFont(&FreeSansBold8pt7b);
      canvas.drawText(canvas.getCursorX(),52,"o"); //Degrees symbol
      //Wind direction icon
      canvas.drawCircle(100+160*i,95,10,0);
      canvas.fillRotatedTriangle(100+160*i, 95, 8, 45*i); //Wind direction examples
      //Rain, wind
      canvas.setFont(&FreeSansBold8pt7b);
      canvas.drawText(45+160*i,110,"12 mm",1);
      canvas.drawText(45+160*i,128,"70%",1);
      canvas.drawText(100+160*i,128,"2 Bft",1);
    }


    canvas.setFont(&FreeSans16pt7b);
    int dayStart = 12;
    for (int j = 0; j<2; j++) {
      for (int i = 0; i <7; i++) {
        canvas.drawRect(1+113*i, 139+170*j, 113, 170, 0);

        int16_t  x1, y1;
        uint16_t w, h;
        canvas.getTextBounds(String(dayStart),10+113*i,170+170*j,&x1,&y1,&w,&h);

        canvas.setCursor(1+113*i+(113-w)/2,170+170*j);
        canvas.print(String(dayStart));
        dayStart++;
      }
    }

    canvas.setFont(&WeatherIcons26pt7b);
    for (int i = 0; i <7; i++) {
      canvas.setCursor(1+113*i,300);
      canvas.print("B");
    }
    canvas.setFont(&FreeSansBold8pt7b);
    for (int i = 0; i <7; i++) {
      canvas.setCursor(50+113*i,284);
      canvas.print("18 24");
      canvas.setCursor(50+113*i,300);
      canvas.print("8 mm");
    }

    // done drawing, so send it off to the display
    EPD_7IN5B_V2_WriteCanvas(_can, 0);

    //A bit confusing, but now: 0 = White, 1 = Red
    canvas.fillScreen(0);  // fill backgrund;

/*
    canvas.setTextColor(1, 0); // Red text, white background
    canvas.setFont(&FreeSans20pt7b);
    canvas.setCursor(100,50); // Position
    canvas.print("24 Dinsdag");  // Print a message

    canvas.setFont(&FreeSansBold8pt7b);
    canvas.setCursor(100,80);
    canvas.print("8 mm November");

    canvas.drawLine(100,50,100,80,1);
*/
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
  digitalWrite(2, HIGH);  // Set LED to high, indicating that we have finished the programm

}

void loop() {
  //Nothing - just a POC so one time execution
}
