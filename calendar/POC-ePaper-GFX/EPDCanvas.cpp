// EPDCanvas is a subclass of GFXcanvas1

#include "EPDCanvas.h"
#include <math.h>

uint16_t EPDCanvas::getWidth(const String &str) {
  int16_t x1;
  int16_t y1;
  uint16_t w;
  uint16_t h;
  getTextBounds(str,200,200,&x1,&y1,&w,&h);
  return w;
}

void EPDCanvas::drawText(int16_t x, int16_t y, const String &str, uint16_t align) {
  if (align==0) {
    setCursor(x,y);
  } else {
    uint16_t w;
    w = getWidth(str);
    if (align==1) {
      setCursor(x-(w/2),y);
    } else {
      setCursor(x-w,y);
    }
  }
  print(str);
}

void EPDCanvas::fillRotatedTriangle(int cx, int cy, int r, float angleDeg) {
  //Calculating alpha, beta, gamma
  //The difference value should be larger than 120 and (much) smaller than 180, we will use 140
  float alpha = angleDeg * DEG_TO_RAD;
  float beta = (angleDeg+140) * DEG_TO_RAD;
  float gamma = (angleDeg-140) * DEG_TO_RAD;

  //Calculating x y for point 0 (cornerpoint A, opposite base-side a)
  float x0 = cx + r * cos(alpha);
  float y0 = cy + r * sin(alpha);

  //Calculating x y for point 1 (cornerpoint B, opposite base-side b)
  float x1 = cx + r * cos(beta);
  float y1 = cy + r * sin(beta);

  //Calculating x y for point 2 (cornerpoint C, opposite base-side b)
  float x2 = cx + r * cos(gamma);
  float y2 = cy + r * sin(gamma);

  //Calculate current center of the triangle
  float gx = (x0+x1+x2)/3;
  float gy = (y0+y1+y2)/3;

  //We want to have the center of the triangle at the cx,cy position, so compensate:
  fillTriangle(x0+cx-gx,y0+cy-gy,x1+cx-gx,y1+cy-gy,x2+cx-gx,y2+cy-gy,0); //Expect 0 as color
}
