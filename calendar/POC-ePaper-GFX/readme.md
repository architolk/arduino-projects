# POC-ePaper-GFX

This example code is inspired by [this example code](https://github.com/tomekness/waveshare_Epd5in83b-adafruitGFX/blob/main/arduino_codeExample/arduino_codeExample.ino). Some changes are made to the Waveshapre EPD_7in5b_V2 driver files to be able to use the AdafruitGFX library instead of the Waveshare's own GUI library from the example.

For this, some changes are made:
- As expected, the code for drawing is based on the AdafruitGFX API;
- A new API function `void EPD_7IN5B_V2_WriteCanvas(GFXcanvas1 *_canvas, UBYTE Block);` is added to write a GFXcanvas1 buffer to the display.

The new API function replaces the EPD_75INB_V2_WritePicture function. This function (as it was derived from the Arduino code base instead of a ESP32 code base) had to deal with the small memory of the Arduino. The original WritePicture function needed to be called four times (twice for B/W, twice for W/R). The new WriteCanvas only needs to be called twice (ones for B/W, ones for W/R), as the memory of the ESP32 is much larger and can handle the canvas buffer in total.

## TODO (1)

The way the Black-White-Red screen works, is that a 0 bit means Black and a 1 bit means White in the B/W mode. But a 0 bit means White and a 1 bit means Red in the W/R mode! To make it all the same, we need to invert one. In the original code, this was done by inverting the drawing and not in the driver code.

On an e-Paper, white is actually the "canvas" color as in normal paper, whereas black is the "canvas" color at a screen that emits light (all other types of screens: LED, TFT, OLED, etc). So I imagine that I should invert the B/W mode, and leave the Red mode as it is.

## TODO (2)
A GFXcanvas1 as a function getBuffer to reach the buffer of the GFXcanvas1. This is actually a bytes buffer (see [GFXcanvas1 constructor](void EPD_7IN5B_V2_WriteCanvas(GFXcanvas1 *_canvas, UBYTE Block);)), so we might be able to read this buffer directly instead of the getPixel function that is used now.
