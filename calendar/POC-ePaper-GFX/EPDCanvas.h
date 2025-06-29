// EPDCanvas is a subclass of GFXCanvas1

#ifndef _EPDCANVAS_H
#define _EPDCANVAS_H

#include <Adafruit_GFX.h>

class EPDCanvas : public GFXcanvas1 {
  public:
    EPDCanvas(uint16_t w, uint16_t h) : GFXcanvas1(w, h, true){};
    uint16_t getWidth(const String &str);
    void drawText(int16_t x, int16_t y, const String &str, uint16_t align = 0); //Align: 0 = left, 1 = middle, 2 = right
    void fillRotatedTriangle(int cx, int cy, int size, float angleDeg);
};
#endif
