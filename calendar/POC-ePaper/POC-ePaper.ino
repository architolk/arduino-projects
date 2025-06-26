#include "src/Config/Debug.h"
#include "src/Config/DEV_Config.h"
#include "src/GUI/GUI_Paint.h"
#include "src/Fonts/fonts.h"
#include <stdlib.h> // malloc() free()

#include "src/e-Paper/EPD_7in5b_V2.h"

//Uncomment this to keep the display state persistant (the display won't be cleared, leaving the last image behind - not recommended if e-Paper is stored)
//#define KEEP_DISPLAY_STATE

void setup() {

  Debug("POC ePaper test\r\n");
  DEV_Module_Init(); //Initialize PINs, SPI bus, Serial

  Debug("e-Paper Init and Clear...\r\n");
  EPD_7IN5B_V2_Init(); //Initialize the display
  EPD_7IN5B_V2_Clear(); //Turn on the display, clear
  DEV_Delay_ms(500);

  //Create a new image cache and fill it with white
  UBYTE *Image;
  boolean memoryOK = true;
  UWORD Imagesize = ((EPD_7IN5B_V2_WIDTH % 8 == 0)? (EPD_7IN5B_V2_WIDTH / 8 ): (EPD_7IN5B_V2_WIDTH / 8 + 1)) * EPD_7IN5B_V2_HEIGHT;
  if((Image = (UBYTE *)malloc(Imagesize / 4)) == NULL) {
      Debug("Failed to apply for memory...\r\n");
      memoryOK = false;
  }

  if (memoryOK) {
    Debug("NewImage:Image\r\n");
    Paint_NewImage(Image, EPD_7IN5B_V2_WIDTH/2, EPD_7IN5B_V2_HEIGHT / 2, 0, WHITE);

    //1.Draw black image
    EPD_7IN5B_V2_Init();
    Paint_SelectImage(Image);
    Paint_Clear(WHITE);
    Paint_DrawPoint(10, 80, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 90, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 100, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 110, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawLine(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(70, 70, 20, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawRectangle(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(80, 70, 130, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(10, 0, "waveshare", &Font16, BLACK, WHITE);
    Paint_DrawNum(10, 50, 987654321, &Font16, WHITE, BLACK);
    EPD_7IN5B_V2_WritePicture(Image, 0);
    Paint_Clear(WHITE);
    EPD_7IN5B_V2_WritePicture(Image, 1);
    //2.Draw red image
    Paint_Clear(WHITE);
    Paint_DrawCircle(160, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(210, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawLine(85, 95, 125, 95, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(105, 75, 105, 115, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawString_EN(10, 20, "hello world", &Font12, WHITE, BLACK);
    Paint_DrawNum(10, 33, 123456789, &Font12, BLACK, WHITE);
    EPD_7IN5B_V2_WritePicture(Image, 2);
    Paint_Clear(WHITE);
    EPD_7IN5B_V2_WritePicture(Image, 3); //3 will turn on the display
    Debug("EPD_Display\r\n");

#ifndef KEEP_DISPLAY_STATE

    Debug("Wait 5 seconds before display is cleared\r\n");
    DEV_Delay_ms(5000);

    Debug("Clear...(to put the e-Paper in it's original clear screen)\r\n");
    EPD_7IN5B_V2_Init();
    EPD_7IN5B_V2_Clear();
#endif

  }

  Debug("Goto Sleep...\r\n");
  EPD_7IN5B_V2_Sleep();
  free(Image); //Only in POC, in the actual usage, we would keep the Image buffer

  // close PWR
  Debug("close PWR, Module enters 0 power consumption ...\r\n");
  DEV_Module_Exit();

}

void loop() {
  //Nothing - just a POC so one time execution
}
