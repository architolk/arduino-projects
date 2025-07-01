#ifndef _EINKDISPLAY_H
#define _EINKDISPLAY_H

#include "DEV_Tools.h"
#include <Adafruit_GFX.h>

// Display resolution
#define EPD_7IN5B_V2_WIDTH       800
#define EPD_7IN5B_V2_HEIGHT      480

#define EPD_BLACK_WHITE_LAYER 0
#define EPD_WHITE_RED_LAYER 1

class EInkDisplay {
  public:
    EInkDisplay(uint8_t pinCS, uint8_t pinDC, uint8_t pinRST, uint8_t pinBUSY, uint8_t pinPWR);
    void setupPins();
    void activate(); //ONLY ONE Display should be active at any time in the routines!!!
    void deactivate();
    void init();
    void sleep();
    void clear();
    void writeCanvas(GFXcanvas1 *_canvas, UBYTE Block);
  protected:
    void EPD_7IN5B_V2_Reset();
    void EPD_7IN5B_V2_SendCommand(UBYTE Reg);
    void EPD_7IN5B_V2_SendData(UBYTE Data);
    void EPD_7IN5B_V2_WaitUntilIdle(void);
    void EPD_7IN5B_V2_TurnOnDisplay();
  private:
    boolean active = false; //Active concerns the state of the display. When inactive, the display receives no power
    uint8_t _pinCS;
    uint8_t _pinDC;
    uint8_t _pinRST;
    uint8_t _pinBUSY;
    uint8_t _pinPWR;
};
#endif
