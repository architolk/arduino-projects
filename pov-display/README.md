# Persistence of view (POV) display

Creating a big POV display, using an ESP32, an AC motor salvaged from a cooling fan and an APA102 LED string.

Typical parts:
- APA102 LED string. These LEDs use a fast SPI bus, necessary for the frame rate of a POV display
- ESP32 controller
- 74AHCT126 level shifter (the ESP32 is 3.3V, the LEDs are 5V)
- Wireless 5V DC-DC transfer (the LEDs are on the rotor, but the voltage source is in the stationary part of the build!)
- Hall sensor (to find out the exact speed of the rotor, thus calculating the frame rate)
