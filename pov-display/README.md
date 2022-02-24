# Persistence of view (POV) display

Creating a big POV display, using an ESP32, an AC motor salvaged from a cooling fan and an APA102 LED string.

Typical parts:
- APA102 LED string. These LEDs use a fast SPI bus, necessary for the frame rate of a POV display
- ESP32 controller
- 74AHCT126 level shifter (the ESP32 is 3.3V, the LEDs are 5V)
- Wireless 5V DC-DC transfer (the LEDs are on the rotor, but the voltage source is in the stationary part of the build!)
- Hall sensor (to find out the exact speed of the rotor, thus calculating the frame rate)

## Resources

https://github.com/im-pro-at/DIYPOV/blob/master/arduino/povdisplay/povdisplay.ino
https://www.instructables.com/Huge-POV-Display/
https://en.wikipedia.org/wiki/Polar_coordinate_system

## How it works

- Connection is made via bluetooth with an android phone
- The App on the android phone can be used to select an image.
- The image is scaled down to 120x120 pixels
- These pixels are transformed to polar Polar coordinates (60 pixels, 3 degrees so 120 frames per rotation)
- They are send over bluetooth serial to the ESP32 (3 bytes per pixel)
- Total storage capacity on the ESP32: 60 x 120 x 3 = 21600 bytes.

## Polar to cartesian

The fun is that we don't want to convert cartesian to polar coordinates, but the other way around. Because we want to send the polor coordinates, we need to understand which X-Y cartesian coordinate this is!

Let theta360 be the theta of the polar coordinates between 0-360 degrees, we wan't a 3 degree precision, so we have an index from 0 - 119. Let r be the radius from 1 to 60 (we have 60 leds, the centre of the picture can't be display as this is te position of the rotor axis).

r = [1..60]
index = [0..119]
theta360 = index * 3
theta = (theta / 180) * Math.PI
x = r * Math.cos(theta)+60
y = r * Math.sin(theta)+60

As the (0,0) position in a bitmap is topleft, we need to shift to the "middle" of the bitmap. So maybe we want 121 pixels?

Notice that we can only show circular pictures: the corners can't be displayed and only the top of a rectangular image will be visible.
